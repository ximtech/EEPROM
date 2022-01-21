#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "I2C_Polling.h"

#ifndef EEPROM_TIMEOUT_MS
#define EEPROM_TIMEOUT_MS 1000
#endif

// EEPROM part numbers are usually designated in k-bits.
typedef enum EEPROMSize {
    EEPROM_2_KBITS = 2,
    EEPROM_4_KBITS = 4,
    EEPROM_8_KBITS = 8,
    EEPROM_16_KBITS = 16,
    EEPROM_32_KBITS = 32,
    EEPROM_64_KBITS = 64,
    EEPROM_128_KBITS = 128,
    EEPROM_256_KBITS = 256,
    EEPROM_512_KBITS = 512,
    EEPROM_1024_KBITS = 1024,
    EEPROM_2048_KBITS = 2048,
} EEPROMSize;

// Commonly used ICs
typedef enum EEPROMType {
    AT24C02,     // Atmel. Size: 2k, Page size: 8 bytes
    AT24C04,     // Atmel. Size: 4k, Page size: 16 bytes
    AT24C08,     // Atmel. Size: 8k, Page size: 16 bytes
    AT24C16,     // Atmel. Size: 16k, Page size: 16 bytes
    AT24C32,     // Atmel. Size: 32k, Page size: 32 bytes
    AT24C64,     // Atmel. Size: 64k, Page size: 32 bytes
    M_24AA02E48, // Microchip. Size: 2k, Page size: 8 bytes
    M_24LC256,   // Microchip. Size: 256k, Page size: 64 bytes
} EEPROMType;

typedef enum EEPROMStatus {
    EEPROM_OK,
    EEPROM_ADDRESS_ERROR,
    EEPROM_WRITE_ERROR,
    EEPROM_READ_ERROR,
} EEPROMStatus;

typedef struct EEPROM_I2C *EEPROM_I2C;

EEPROM_I2C initByTypeEEPROM(I2C_TypeDef *I2Cx, EEPROMType type, uint8_t deviceAddress);
EEPROM_I2C initEEPROM(I2C_TypeDef *I2Cx, EEPROMSize size, uint16_t pageSize, uint8_t deviceAddress);
I2CStatus beginEEPROM(EEPROM_I2C eeprom);// do a dummy write (no data sent) to the device so that the caller can determine whether it is responding.

EEPROMStatus writeBytesEEPROM(EEPROM_I2C eeprom, uint32_t address, uint8_t *bytes, uint16_t length);
EEPROMStatus writeByteEEPROM(EEPROM_I2C eeprom, uint32_t address, uint8_t value);

EEPROMStatus readBytesEEPROM(EEPROM_I2C eeprom, uint32_t address, uint8_t *bytes, uint16_t length);
uint8_t readByteEEPROM(EEPROM_I2C eeprom, uint32_t address);
EEPROMStatus updateEEPROM(EEPROM_I2C eeprom, uint32_t address, uint8_t value);

void deleteEEPROM(EEPROM_I2C eeprom);