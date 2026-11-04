# FLIR-Hadron-640

Experimenting using Teledyne FLIR Hadron 640R.

**Not affiliated with Teledyne FLIR.**

## Build and run

- Developed using qt 6.11.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```
```bash
cmake --build build -j$(nproc)
```
```bash
./build/flir_hadron_640
```

