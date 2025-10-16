#include "kernel_operator.h"

using namespace AscendC;

constexpr uint32_t TILE_NUM = 4;
constexpr uint32_t BUFFER_NUM = 2;

class KernelAddCustom {
public:
  __aicore__ inline KernelAddCustom() {}

  __aicore__ inline void Init(GM_ADDR x, GM_ADDR y, GM_ADDR z,
                              uint32_t totalLength) {
    this->blockLength = totalLength / GetBlockNum();
    this->tileLength = this->blockLength / TILE_NUM / BUFFER_NUM;

    xGm.SetGlobalBuffer((__gm__ half *)x + this->blockLength * GetBlockIdx(),
                        this->blockLength);
    yGm.SetGlobalBuffer((__gm__ half *)y + this->blockLength * GetBlockIdx(),
                        this->blockLength);
    zGm.SetGlobalBuffer((__gm__ half *)z + this->blockLength * GetBlockIdx(),
                        this->blockLength);

    pipe.InitBuffer(inQueueX, BUFFER_NUM, this->tileLength * sizeof(half));
    pipe.InitBuffer(inQueueY, BUFFER_NUM, this->tileLength * sizeof(half));
    pipe.InitBuffer(outQueueZ, BUFFER_NUM, this->tileLength * sizeof(half));
  }

  __aicore__ inline void Process() {
    int32_t loopCount = TILE_NUM * BUFFER_NUM;
    for (int32_t i = 0; i < loopCount; i++) {
      CopyIn(i);
      Compute(i);
      CopyOut(i);
    }
  }

private:
  __aicore__ inline void CopyIn(int32_t progress) {
    LocalTensor<half> xLocal = inQueueX.AllocTensor<half>();
    LocalTensor<half> yLocal = inQueueY.AllocTensor<half>();
    DataCopy(xLocal, xGm[progress * this->tileLength], this->tileLength);
    DataCopy(yLocal, yGm[progress * this->tileLength], this->tileLength);
    inQueueX.EnQue(xLocal);
    inQueueY.EnQue(yLocal);
  }

  __aicore__ inline void Compute(int32_t progress) {
    LocalTensor<half> xLocal = inQueueX.DeQue<half>();
    LocalTensor<half> yLocal = inQueueY.DeQue<half>();
    LocalTensor<half> zLocal = outQueueZ.AllocTensor<half>();
    Add(zLocal, xLocal, yLocal, this->tileLength);
    inQueueX.FreeTensor(xLocal);
    inQueueY.FreeTensor(yLocal);
    outQueueZ.EnQue(zLocal);
  }

  __aicore__ inline void CopyOut(int32_t progress) {
    LocalTensor<half> zLocal = outQueueZ.DeQue<half>();
    DataCopy(zGm[progress * this->tileLength], zLocal, this->tileLength);
    outQueueZ.FreeTensor(zLocal);
  }

private:
  TPipe pipe;
  TQue<QuePosition::VECIN, BUFFER_NUM> inQueueX, inQueueY;
  TQue<QuePosition::VECOUT, BUFFER_NUM> outQueueZ;

  GlobalTensor<half> xGm;
  GlobalTensor<half> yGm;
  GlobalTensor<half> zGm;

  uint32_t blockLength;
  uint32_t tileLength;
};

extern "C" __global__ __aicore__ void
add_custom(GM_ADDR x, GM_ADDR y, GM_ADDR z, uint32_t totalLength) {
  KernelAddCustom op;
  op.Init(x, y, z, totalLength);
  op.Process();
}
