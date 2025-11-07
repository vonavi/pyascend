PyAscend
========

A lightweight Python wrapper that simplifies copying data to and from the Ascend NPU and launching kernels on it. Currently tested on **Ascend 910B4**.

## Building

The project can be built as a regular Python package, but we also provide an alternative, **reproducible** build method using [Nix flakes](https://wiki.nixos.org/wiki/Flakes). We focus here on the Nix-based workflow.

First, install the [Ascend CANN toolkit](https://www.hiascend.com/en/software/cann/community) and add it as a Nix flake input. For example:
```console
$ nix flake lock --override-input ascendcDevkitPath \
      "path:${HOME}/Ascend/ascend-toolkit/8.3.RC1.alpha002/aarch64-linux"
```
Then, build the project:
```console
$ nix build
```

## Testing

Enter the development environment:
```console
$ nix develop
$ PYTHONPATH=result/lib/python3.12/site-packages:$PYTHONPATH
```
Inside it, run a test for the built Ascend kernel:
```console
$ pytest result/lib/python3.12/site-packages/pyascend/tests/test_add_custom.py
```
