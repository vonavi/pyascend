```console
$ nix flake lock --override-input ascendcDevkitPath \
      "path:${HOME}/Ascend/ascend-toolkit/8.3.RC1.alpha002/aarch64-linux"
```

```console
$ nix develop
$ PYTHONPATH=result/lib/python3.12/site-packages:$PYTHONPATH
```

```console
$ nix build
```

```console
$ pytest result/lib/python3.12/site-packages/pyascend/tests/test_add_custom.py
```
