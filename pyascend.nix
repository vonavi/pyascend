{
  buildPythonPackage,

  # Native build inputs
  cmake,
  ninja,
  pybind11,
  scikit-build-core
}:

buildPythonPackage {
  pname = "pyascend";
  version = "0.0.1";
  pyproject = true;

  src = ./.;

  nativeBuildInputs = [
    cmake
    ninja
    pybind11
    scikit-build-core
  ];

  dontUseCmakeConfigure = true;

  pythonImportsCheck = [ "pyascend" ];
}
