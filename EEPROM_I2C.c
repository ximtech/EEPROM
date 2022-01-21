#include "EEPROM_I2C.h"

#define ONE_BYTE 1
#define TWO_BYTES 2
#define DUMMY_BYTE 0

#define WRITE_DELAY_US 500

typedef enum EEPROMPageSize {
    EEPROM_PAGE_SIZE_8_BYTES = 8,
    EEPROM_PAGE_SIZE_16_BYTES = 16,
    EEPROM_PAGE_SIZE_32_BYTES = 32,
    EEPROM_PAGE_SIZE_64_BYTES = 64,
} EEPROMPageSize;

struct EEPROM_I2C {
    I2C_Polling i2c;
    uint8_t deviceAddress;
    uint16_t pageSizeBytes;
    uint8_t chipSelectBitShift;     // number of bits to shift address for chip select bits in control byte
    uint16_t numberOfAddressBytes;  // number of address bytes (1 or 2)
    uint32_t totalCapacity;
};

static EEPROMSize getEEPROMSizeByType(EEPROMType type);
static EEPROMPageSize getEEPROMPageSize(EEPROMType type);
static uint8_t calculateBitShift(EEPROMSize deviceCapacity);


EEPROM_I2C initByTypeEEPROM(I2C_TypeDef *I2Cx, EEPROMType type, uint8_t deviceAddress) {
    EEPROMSize size = getEEPROMSizeByType(type);
    EEPROMPageSize pageSize = getEEPROMPageSize(type);
    return initEEPROM(I2Cx, size, pageSize, deviceAddress);
}

EEPROM_I2C initEEPROM(I2C_TypeDef *I2Cx, EEPROMSize size, uint16_t pageSize, uint8_t deviceAddress) {
    EEPROM_I2C eeprom = malloc(sizeof(struct EEPROM_I2C));
    if (eeprom != NULL) {
        eeprom->i2c = initI2C(I2Cx, I2C_ADDRESSING_MODE_7BIT, EEPROM_TIMEOUT_MS);
        eeprom->deviceAddress = deviceAddress;
        eeprom->pageSizeBytes = pageSize;
        eeprom->chipSelectBitShift = calculateBitShift(size);
        eeprom->numberOfAddressBytes = size > EEPROM_16_KBITS ? TWO_BYTES : ONE_BYTE;
        eeprom->totalCapacity = (size * 1024UL) / 8;
    }
    return eeprom;
}

I2CStatus beginEEPROM(EEPROM_I2C eeprom) {
    I2CStatus status = startAsMasterI2C(&eeprom->i2c, eeprom->deviceAddress, I2C_WRITE_TO_SLAVE);
    if (status != I2C_OK) return status;

    if (eeprom->numberOfAddressBytes == TWO_BYTES) {
        transmitByteAsMasterI2C(&eeprom->i2c, DUMMY_BYTE);
    }
    transmitByteAsMasterI2C(&eeprom->i2c, DUMMY_BYTE);
    return stopAsMasterI2C(&eeprom->i2c);
}

EEPROMStatus writeBytesEEPROM(EEPROM_I2C eeprom, uint32_t address, uint8_t *bytes, uint16_t length) {
    if (address + length > eeprom->totalCapacity) { // check if space enough
        return EEPROM_ADDRESS_ERROR;
    }

    while (length > 0) {
        uint16_t remainingBytesOnPage = eeprom->pageSizeBytes - (address & (eeprom->pageSizeBytes - 1));  // number of bytes remaining on current page, starting at address
        uint16_t bytesToSend = length < remainingBytesOnPage ? length : remainingBytesOnPage;   // number of bytes to write
        uint8_t controlByte = eeprom->deviceAddress | (uint8_t) (address >> eeprom->chipSelectBitShift); // control byte (I2C device address & chip/block select bits)

        startAsMasterI2C(&eeprom->i2c, controlByte, I2C_WRITE_TO_SLAVE);
        if (eeprom->numberOfAddressBytes == TWO_BYTES) {
            transmitByteAsMasterI2C(&eeprom->i2c, address >> 8); // high address byte
        }
        transmitByteAsMasterI2C(&eeprom->i2c, address); //low address byte

        for (uint16_t i = 0; i < bytesToSend; i++) {    // sequential write
            transmitByteAsMasterI2C(&eeprom->i2c, bytes[i]);
        }
        I2CStatus status = stopAsMasterI2C(&eeprom->i2c);
        if (status != I2C_OK) {
            return EEPROM_WRITE_ERROR;
        }

        for (uint8_t i = 0; i < 100; i++) { // wait up to 50ms for the write to complete
            delay_us(WRITE_DELAY_US);
            status = beginEEPROM(eeprom);   // check status
            if (status == I2C_OK) {
                break;
            }
        }
        if (status != I2C_OK) {
            return EEPROM_WRITE_ERROR;
        }

        address += bytesToSend; // increment the EEPROM address
        bytes += bytesToSend;   // increment the input data pointer
        length -= bytesToSend;   // decrement the number of bytes left to write
    }
    return EEPROM_OK;
}

EEPROMStatus writeByteEEPROM(EEPROM_I2C eeprom, uint32_t address, uint8_t value) {
    return writeBytesEEPROM(eeprom, address, &value, 1);
}

EEPROMStatus readBytesEEPROM(EEPROM_I2C eeprom, uint32_t address, uint8_t *bytes, uint16_t length) {
    if (address + length > eeprom->totalCapacity) {
        return EEPROM_ADDRESS_ERROR;
    }

    while (length > 0) {
        uint16_t remainingBytesOnPage = eeprom->pageSizeBytes - (address & (eeprom->pageSizeBytes - 1));  // number of bytes remaining on current page, starting at address
        uint16_t bytesToSend = length < remainingBytesOnPage ? length : remainingBytesOnPage;   // number of bytes to write
        uint8_t controlByte = eeprom->deviceAddress | (uint8_t) (address >> eeprom->chipSelectBitShift); // control byte (I2C device address & chip/block select bits)

        startAsMasterI2C(&eeprom->i2c, controlByte, I2C_WRITE_TO_SLAVE);
        if (eeprom->numberOfAddressBytes == TWO_BYTES) {
            transmitByteAsMasterI2C(&eeprom->i2c, address >> 8); // high address byte
        }
        transmitByteAsMasterI2C(&eeprom->i2c, address); //low address byte
        I2CStatus status = stopAsMasterI2C(&eeprom->i2c);
        if (status != I2C_OK) {
            return EEPROM_READ_ERROR;
        }

        startAsMasterI2C(&eeprom->i2c, controlByte, I2C_READ_FROM_SLAVE);
        for (uint16_t i = 0; i < length; i++) {
            if (i == (length - 1)) { // for last byte receive value with NACK
                receiveByteAsMasterWithNackI2C(&eeprom->i2c, &bytes[i]);
            } else {
                receiveByteAsMasterI2C(&eeprom->i2c, &bytes[i]);
            }
        }

        address += bytesToSend; // increment the EEPROM address
        bytes += bytesToSend;   // increment the input data pointer
        length -= bytesToSend;   // decrement the number of bytes left to write
    }
    return EEPROM_OK;
}

uint8_t readByteEEPROM(EEPROM_I2C eeprom, uint32_t address) {
    uint8_t byte;
    EEPROMStatus status = readBytesEEPROM(eeprom, address, &byte, 1);
    return status == EEPROM_OK ? byte : 0xFF;
}

EEPROMStatus updateEEPROM(EEPROM_I2C eeprom, uint32_t address, uint8_t value) {
    uint8_t previousValue = readByteEEPROM(eeprom, address);
    if (previousValue == value) {
        return EEPROM_OK;
    }
    return writeByteEEPROM(eeprom, address, value);
}

void deleteEEPROM(EEPROM_I2C eeprom) {
    free(eeprom);
}

static EEPROMSize getEEPROMSizeByType(EEPROMType type) {
    switch (type) {
        case AT24C02:
            return EEPROM_2_KBITS;
        case AT24C04:
            return EEPROM_4_KBITS;
        case AT24C08:
            return EEPROM_8_KBITS;
        case AT24C16:
            return EEPROM_16_KBITS;
        case AT24C32:
            return EEPROM_32_KBITS;
        case AT24C64:
            return EEPROM_64_KBITS;
        case M_24AA02E48:
            return EEPROM_2_KBITS;
        case M_24LC256:
            return EEPROM_256_KBITS;
        default:
            return 0;
    }
}

static EEPROMPageSize getEEPROMPageSize(EEPROMType type) {
    switch (type) {
        case AT24C02:
        case M_24AA02E48:
            return EEPROM_PAGE_SIZE_8_BYTES;
        case AT24C04:
        case AT24C08:
        case AT24C16:
            return EEPROM_PAGE_SIZE_16_BYTES;
        case AT24C32:
        case AT24C64:
            return EEPROM_PAGE_SIZE_32_BYTES;
        case M_24LC256:
            return EEPROM_PAGE_SIZE_64_BYTES;
        default:
            return 0;
    }
}

// determine the bitshift needed to isolate the chip select bits from the address to put into the control byte
static uint8_t calculateBitShift(EEPROMSize deviceCapacity) {
    switch (deviceCapacity) {
        case EEPROM_2_KBITS:
        case EEPROM_4_KBITS:
        case EEPROM_8_KBITS:
        case EEPROM_16_KBITS:
            return 8;
        case EEPROM_32_KBITS:
        case EEPROM_64_KBITS:
            return 12;
        case EEPROM_128_KBITS:
            return 13;
        case EEPROM_256_KBITS:
            return 14;
        case EEPROM_512_KBITS:
        case EEPROM_1024_KBITS:
        case EEPROM_2048_KBITS:
            return 16;
        default:
            return 0;
    }
}