RP2040 KBM Jiggler
===

Jiggles your keyboard and mouse like you have never seen before. Currently tested to work with RP2040-Zero.

Requirements
-------

- arm-none-eabi-gcc
- pico-sdk (1.5.1)
- tinyusb (0.15.0)

Build
-------

```sh
git submodule update --init
cmake -B build && cmake --build build/ --config Release
```

Installation
-------

Copy UF2 file into RPI-RP2.

License
-------

MIT
