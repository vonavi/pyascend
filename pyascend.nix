{
  ascendcDevkitPath,
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

  CMAKE_ARGS = [
    "-DASCENDC_DEVKIT_PATH=${ascendcDevkitPath}"
  ];

  pythonImportsCheck = [ "pyascend" ];
}
