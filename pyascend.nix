{
  ascendcDevkitPath,
  buildPythonPackage,
  lib,
  python,

  # Native build inputs
  autoPatchelfHook,
  cmake,
  ninja,
  pybind11,
  scikit-build-core,

  # Propagated build inputs
  numpy,
  pytest,

  # Boolean flags
  enableTests ? false
}:

buildPythonPackage rec {
  pname = "pyascend";
  version = "0.0.1";
  pyproject = true;

  src = ./.;

  nativeBuildInputs = [
    autoPatchelfHook
    cmake
    ninja
    pybind11
    scikit-build-core
  ];

  propagatedBuildInputs = lib.optionals enableTests [
    numpy
    pytest
  ];

  dontUseCmakeConfigure = true;

  CMAKE_ARGS = [
    "-DASCENDC_DEVKIT_PATH=${ascendcDevkitPath}"
    (lib.cmakeBool "ENABLE_TESTS" enableTests)
  ];

  preFixup = ''
    addAutoPatchelfSearchPath "${ascendcDevkitPath}/lib64"
  '';

  postInstall = lib.optionalString enableTests ''
    cp -r tests "$out/${python.sitePackages}/${pname}"
  '';
}
