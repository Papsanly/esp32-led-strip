# esp32 led strip example project

## esp32 nix development shell

```bash
nix develop github:mirrexagon/nixpkgs-esp-dev#esp-idf-xtensa
```

## build, flash and monitor

Run inside nix development shell:

```bash
idf.py build && idf.py flash && idf.py monitor
```
