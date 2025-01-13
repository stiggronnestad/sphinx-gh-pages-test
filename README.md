# Firmware Power Management Unit (PMU)
Firmware for all devices in the Power Management Unit (PMU).

- [Firmware Power Management Unit (PMU)](#firmware-power-management-unit-pmu)
  - [Overview Repository](#overview-repository)

## Overview Repository

| Folder                                        | Device                     | Description                                                   |
| --------------------------------------------- | -------------------------- | ------------------------------------------------------------- |
| [/boost-converter](boost-converter/README.md) | Boost Converter            | Boost converter for the PMU (up to 2 devices).                |
| [/ccu](ccu/README.md)                         | Central Control Unit (CCU) | Main central control unit for the PMU.                        |
| [/dcdc](dcdc/README.md)                       | DC/DC Converter            | DC/DC converter for the PMU (up to 2 devices, BMS and EV).    |
| [/documentation](documentation/README.md)     | Documentation              | Documentation for the PMU.                                    |
| [/inverter](inverter/README.md)               | Inverter                   | Inverter for the PMU.                                         |
| [/libs](libs/README.md)                       | Libraries/includes         | Libraries/include files for the different devices of the PMU. |


:x: Never ignore source code, documentation and build-configurations that are platform agnostic.

:white_check_mark: Always ignore files produced by the build-process itself.
