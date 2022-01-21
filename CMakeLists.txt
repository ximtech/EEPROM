cmake_minimum_required(VERSION 3.20)
project(EEPROM C)

set(CMAKE_C_STANDARD 99)

include(cmake/CPM.cmake)

CPMAddPackage(
        NAME STM32Core
        GITHUB_REPOSITORY ximtech/STM32Core
        GIT_TAG origin/main)

set(EEPROM_DIRECTORY
        ${I2C_POLLING_DIRECTORY}
        ${EEPROM_SOURCE_DIR}
        ${EEPROM_SOURCE_DIR}/include
        CACHE STRING "EEPROM directories include to the main project" FORCE)

set(EEPROM_SOURCES
        ${I2C_POLLING_SOURCES}
        ${EEPROM_SOURCE_DIR}/EEPROM_I2C.c
        ${EEPROM_SOURCE_DIR}/include/EEPROM_I2C.h
        CACHE STRING "EEPROM source files include to the main project" FORCE)