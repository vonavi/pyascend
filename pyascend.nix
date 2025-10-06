{
  buildPythonPackage,

  # Native build inputs
  poetry-core,
  pybind11,
  setuptools
}:

buildPythonPackage {
  pname = "pyascend";
  version = "0.0.1";
  pyproject = true;

  src = ./.;

  nativeBuildInputs = [
    poetry-core
    pybind11
    setuptools
  ];

  pythonImportsCheck = [ "pyascend" ];
}
