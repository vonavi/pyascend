{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.05";
    flake-utils.url = "github:numtide/flake-utils";
    ascendcDevkitPath.flake = false;
  };

  outputs = { self, nixpkgs, flake-utils, ascendcDevkitPath }:
  flake-utils.lib.eachDefaultSystem (system:
    let
      pkgs = import nixpkgs { inherit system; };
      python = pkgs.python3;
      pyascend = python.pkgs.callPackage ./pyascend.nix {
        inherit ascendcDevkitPath;
        enableTests = true;
      };
    in {
      defaultPackage = python.withPackages (_: [ pyascend ]);

      devShell = pkgs.mkShell {
        buildInputs = with pkgs; [
          clang-tools
          ruff
        ];
        inputsFrom = [ pyascend ];
      };
    });
}
