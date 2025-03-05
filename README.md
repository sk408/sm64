# Pico-ASHA: Audio Streaming for Hearing Aids on Raspberry Pi Pico W

This project implements the Android's Audio Streaming for Hearing Aids (ASHA) protocol on the Raspberry Pi Pico W microcontroller. It enables wireless audio streaming from a computer to compatible hearing aids using Bluetooth Low Energy (BLE).

## Overview

Pico-ASHA turns your Raspberry Pi Pico W into a wireless audio transmitter for hearing aids, allowing you to stream audio directly from your computer to your hearing aids without additional hardware.

## Features

- USB audio interface (appears as a sound card to the host computer)
- Bluetooth Low Energy (BLE) connection to compatible hearing aids
- Implementation of the ASHA protocol 
- Dual-core operation (Core 0 for USB/audio, Core 1 for Bluetooth)
- G.722 audio encoding
- Volume control
- LED status indicators
- Logging system via USB serial

## Requirements

- Raspberry Pi Pico W microcontroller
- Raspberry Pi Pico SDK (v2.1.1 or newer)
- C++ compiler supporting C++20
- CMake (3.13 or newer)
- Git for dependency management
- ASHA-compatible hearing aids

## Building the Project

1. Clone this repository:
   ```
   git clone https://github.com/sk408/sm64.git
   cd sm64
   ```
   
2. Initialize and update the submodules:
   ```
   git submodule update --init --recursive
   ```
   
3. Create a build directory:
   ```
   mkdir build
   cd build
   ```
   
4. Configure with CMake:
   ```
   cmake ..
   ```
   
5. Build the project:
   ```
   make
   ```
   
6. The output will be a `pico_asha.uf2` file that can be flashed to the Raspberry Pi Pico W.

## Usage

1. Flash the `pico_asha.uf2` file to your Raspberry Pi Pico W
2. Connect the Pico W to your computer via USB
3. Put your hearing aids in pairing mode
4. The Pico W will appear as a USB audio device on your computer
5. Play audio through the Pico-ASHA device to stream to your hearing aids
6. The built-in LED will indicate connection and streaming status

## Project Structure

- `src/` - Main source code directory
- `lib/` - External libraries and dependencies
- `patch/` - Patches for external dependencies
- `doc/` - Documentation
- `CMakeLists.txt` - Build configuration
- `pico_sdk_import.cmake` - Pico SDK integration script

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

[MIT License](LICENSE)

## Contact

- GitHub: [sk408](https://github.com/sk408)
- Repository: [sm64](https://github.com/sk408/sm64) 