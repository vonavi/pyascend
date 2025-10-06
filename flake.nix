{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
  flake-utils.lib.eachDefaultSystem (system:
    let
      pkgs = import nixpkgs { inherit system; };
      python = pkgs.python3;
      pyascend = python.pkgs.callPackage ./pyascend.nix { };
    in {
      defaultPackage = python.withPackages (_: [ pyascend ]);

      devShell = pkgs.mkShell {
        buildInputs = with pkgs; [
          poetry
          ruff
        ];
        inputsFrom = [ pyascend ];
      };
    });
}
