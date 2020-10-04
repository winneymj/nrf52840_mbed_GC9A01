# Project for Adafruit Feather nRF52840 Express and round GC9A01 display
BOM:
- Adafruit Feather nRF52840 Express - (https://www.adafruit.com/product/4062)
- 1.28'' IPS TFT LCD Display 240x240 Round Circle Screen for Smart Watch - (https://www.buydisplay.com/1-28-inch-tft-lcd-display-240x240-round-circle-screen-for-smart-watch)

# Building this repo
## Prerequisite
This assumes you have the mbed CLI installed and are able to build and compile using the ARM GCC.
## Steps
- Clone repo
- cd into cloned directory
- Open CMD or powershell window and enter command below to clone mbed-os, lvgl and lvgl_drivers
```text
mbed update
```
- Rename the lv_drivers/GC9A01.c to GC9A01.cpp.  This is because the driver needs to include the **mbed.h** header file and only C++ files can do this.
- compile the codeby entering command
```text
mbed compile -t GCC_ARM -m ADAFRUIT_NRF52840_FEATHER
```
## Flashing to Adafruit board 
Flashing the code to the board requires using the **adafruit-nrfutil**.

**prerequisites:**

- python3

**Installation (Windows only):**

- Install adafruit-nrfutil
```text
pip3 install --user adafruit-nrfutil
```
- Locate the adafruit-nrfutil.exe and add to path
- Convert hex file to DFU zip by replacing &lt;clone&gt; with clone directory name and entering command:
```text
adafruit-nrfutil dfu genpkg --dev-type 0x0052 --sd-req 0x00B6 --application .\BUILD\ADAFRUIT_NRF52840_FEATHER\GCC_ARM\<clone>_application.hex .\BUILD\ADAFRUIT_NRF52840_FEATHER\GCC_ARM\<clone>_application.hex.zip
```
- Flash zip file to board using command:
```text
adafruit-nrfutil --verbose dfu serial -pkg .\BUILD\ADAFRUIT_NRF52840_FEATHER\GCC_ARM\<clone>_application.hex.zip -p COM7 -b 115200 --singlebank
```
## Building and Flash board in Visual Studio Code
**To build and flash under Visual Studio Code**
- update Visual Studio Code tasks.json to include following configuration inside the **tasks** array:
```json
    "tasks": [
        {
            "label": "compile release",
            "group": "build",
            "command": "mbed",
            "args": [
                "compile",
                "--profile=release",
                "-v",
                "-t",
                "GCC_ARM",
                "-m",
                "ADAFRUIT_NRF52840_FEATHER"
            ]
        },
        {
            "label": "ada-nrfutil app hex to DFU zip",
            "group": "build",
            "command": "adafruit-nrfutil",
            "args": [
                "dfu",
                "genpkg",
                "--dev-type",
                "0x0052",
                "--sd-req",
                "0x00B6",
                "--application",
                "${workspaceFolder}\\BUILD\\ADAFRUIT_NRF52840_FEATHER\\GCC_ARM-RELEASE\\${workspaceFolderBasename}_application.hex",
                "${workspaceFolder}\\BUILD\\ADAFRUIT_NRF52840_FEATHER\\GCC_ARM-RELEASE\\${workspaceFolderBasename}_application.hex.zip"
            ],
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared",
                "showReuseMessage": true,
                "clear": false
            },
        },
        {
            "label": "ada-nrfutil flash DFU pkg",
            "group": "build",
            "command": "adafruit-nrfutil",
            "args": [
                "--verbose",
                "dfu",
                "serial",
                "-pkg",
                "${workspaceFolder}\\BUILD\\ADAFRUIT_NRF52840_FEATHER\\GCC_ARM-RELEASE\\${workspaceFolderBasename}_application.hex.zip",
                "-p",
                "COM7",
                "-b",
                "115200",
                "--singlebank"
            ]
        },
        {
            "label": "Build Release",
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "dependsOrder": "sequence",
            "dependsOn": ["compile release", "ada-nrfutil app hex to DFU zip", "ada-nrfutil flash DFU pkg"]
        }
    ]
``` 

## Compiling and Building

- Connect Adafruit board to computer via USB cable.
- Check the virtual COM port it opened (on my laptop was COM7) but may be different on yours. If it is not COM7 then make sure to change the tasks.json file **"ada-nrfutil flash DFU pkg"** section port to match.
- In Visual Studio Code execute a build. (SHIFT + CTRL + B).
- Whilst waiting for build press the reset button on Adafruit twice quickly to enter DFU programming mode.  Neopixel LED will be green.
- Watch Console output for compile, hex to DFU zip and eventual upload to board.

## Enabling console printf back to host computer via USB

To use debug printf statements and see them on your host computer you need to include into your main.cpp the header file **USBConsole.h** as below:
```c
#include "USBConsole.h"
```  
you can then use the standard printf:
```c
printf("main: ENTER\r\n");
```
To view the output you will need to use a terminal/telnet client such as Putty.  When the Adafruit board is running with the USBConsole enabled program a virtual COM port will be opened (COM8 on my computer) and the output streamed.  The program suspends until the COM port is opened....so remember to do this if you have used the USBConsole, otherwise it will look like you program is not working.

You can remove the header file (USBConsole.h)...and leave the printf's in place to allow the program to run as normal and not suspend, and of course without the output.

