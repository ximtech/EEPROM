# EEPROM

Refactored Arduino library from [JChristensen](https://github.com/JChristensen/JC_EEPROM) to STM32 LL(Low-Layer) C library.

### Features

- Predefined commonly used EEPROM chip types for easier configuration.
- Fast read and write operations
- Auto memory paging resolving
- No HAL dependencies

<img src="https://github.com/ximtech/EEPROM/blob/main/example/view.PNG" alt="image" width="300"/>

### Add as CPM project dependency

How to add CPM to the project, check the [link](https://github.com/cpm-cmake/CPM.cmake)

```cmake
CPMAddPackage(
        NAME EEPROM
        GITHUB_REPOSITORY ximtech/EEPROM
        GIT_TAG origin/main)
```

### Project configuration

1. Start project with STM32CubeMX:
    * [I2C configuration](https://github.com/ximtech/EEPROM/blob/main/example/config.PNG)
2. Select: Project Manager -> Advanced Settings -> I2C -> LL
3. Generate Code
4. Add sources to project:
```cmake
add_subdirectory(${STM32_CORE_SOURCE_DIR}/I2C/Polling)  # add I2C to project

include_directories(${EEPROM_DIRECTORY})   # source directories
file(GLOB_RECURSE SOURCES ${EEPROM_SOURCES})    # source files
```

3. Then Build -> Clean -> Rebuild Project

### Wiring

- <img src="https://github.com/ximtech/EEPROM/blob/main/example/wiring.PNG" alt="image" width="300"/>

### Usage

***Data write and read from EEPROM***
```c
#include "EEPROM_I2C.h"

#define EEPROM_I2C_ADDRESS 0xA0
#define EEPROM_VALUE_ADDRESS 0
#define VALUE_TO_SAVE 55

int main() {
    
    EEPROM_I2C eeprom = initByTypeEEPROM(I2C1, AT24C04, EEPROM_I2C_ADDRESS);
    I2CStatus status = beginEEPROM(eeprom);
    if (status != I2C_OK) {
        return -1;  // failed to start, check status for more info
    }
    
    EEPROMStatus eepromStatus = writeByteEEPROM(eeprom, EEPROM_VALUE_ADDRESS, VALUE_TO_SAVE);
    uint8_t value = readByteEEPROM(eeprom, EEPROM_VALUE_ADDRESS);
    printf("%d\n", value);
}
```