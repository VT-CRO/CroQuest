# Embedded Code

This repository contains all the necessary code for hardware programming on the ESP32.

To simplify library management and avoid configuration issues, all development is consolidated within a single PlatformIO project directory rather than creating separate projects for each module.

The full `ESP32` project folder is included, with new implementation files organized under the `Files` subdirectory. Inside Files, the `hardware/` folder contains low-level code specific to the CroQuest hardware (e.g., Bluetooth, SD card), while the `software/` folder includes higher-level features like game logic, UI menus, and more.


Tree Structure of Directories 

```
embedded_code 
├── README.md
└── ESP32
    ├── Files
    │   ├── hardware
    │   │   ├── Bluetooth_Master.cpp
    │   │   ├── Bluetooth_Slave.cpp
    │   │   └── SD_working.cpp
    │   └── software
    │       └── menu.cpp
    ├── include
    │   └── README
    ├── lib
    │   └── README
    ├── platformio.ini
    ├── src
    │   └── main.cpp
    └── test
        └── README

9 directories, 10 files


```
