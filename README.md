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

# Boost Converter/MPPT

Firmware for the Boost Converter/MPPT.

- [Boost Converter/MPPT](#boost-convertermppt)
  - [Structure](#structure)
    - [Folders](#folders)
    - [.gitignore](#gitignore)
    - [Commits / Branches](#commits--branches)
    - [Conventions](#conventions)
    - [](#)
  - [CMake](#cmake)
    - [Headers](#headers)
    - [Sources](#sources)

## Structure

This is a STM32/CubeMX project with the CMake build system.

### Folders

| Folder  | Description                                                    | Originator | gitignore |
| ------- | -------------------------------------------------------------- | ---------- | --------- |
| .vscode | Visual Studio Code settings (gitignored, setup is documented). | User       | Yes       |
| build   | CMake build directory.                                         | CMake      | Yes       |
| cmake   | CMake scripts.                                                 | CubeMX     | No        |
| Core    | Core files.                                                    | CubeMX     | No        |
| docs    | Documentation for the specific component/device.               | User       | No        |
| Drivers | STM32CubeMX generated drivers.                                 | CubeMX     | No        |
| libs    | External libraries (firmware-libraries as sub-module).         | User       | No        |
| src     | User source files.                                             | User       | No        |

### .gitignore

:x: Never ignore source code, documentation and build-configurations that are platform agnostic.

:white_check_mark: Always ignore files produced by the build-process itself.

:white_check_mark: Always ignore local configuration files for IDE/tools.

:white_check_mark: Always ignore files containing sensitive information (secrets, api-keys, etc).

### Commits / Branches

:x: Never keep sensitive information in the source-code or commit messages.

:x: Never commit code directly to the 'main' branch.

:white_check_mark: Always work in a separate feat/fix branch to avoid conflicts.

:white_check_mark: Always create pull-requests and request reviews from other-team members.

:white_check_mark: Always create version tags for releases.

:heavy_exclamation_mark: In general the 'main' branch will be guarded by a ruleset that requires status-checks, linear history and pull-requests with code-review before squash-merging. This is to ensure that the code is reviewed by at least one other team member before being merged into the main branch. :heavy_exclamation_mark:

### Conventions

- [Repository Naming Conventions](https://evert-as.atlassian.net/wiki/spaces/Firmware/pages/173899777/Repository+Naming+Conventions)


### 


## CMake

### Headers

Header files must be included in the `CMakeLists.txt` file.

```cmake
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    libs/firmware-libraries/can_handler/src/
    libs/firmware-libraries/device/src/
    libs/firmware-libraries/evert_hal/src/
    libs/firmware-libraries/status_register/src/
    libs/firmware-libraries/task_scheduler/src/
    src/
    # DSP added
    Drivers/CMSIS/DSP/Include
)
```

### Sources

Source files must be included in the `CMakeLists.txt` file.

```cmake
# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    libs/firmware-libraries/can_handler/src/can_handler.c
    libs/firmware-libraries/device/src/evert_device.c
    libs/firmware-libraries/evert_hal/src/debugging.c
    libs/firmware-libraries/evert_hal/src/evert_hal_dwt.c
    libs/firmware-libraries/evert_hal/src/evert_hal.c
    libs/firmware-libraries/status_register/src/status_register.c
    libs/firmware-libraries/task_scheduler/src/task_scheduler.c
)
```

# GitOps

## Quick References

[Github/Evert/Firmware-*](https://github.com/orgs/InSol-Tech/repositories?language=&q=firmware-&sort=&type=all)

## Status

| Repository                                                           | Build                                                                                                                                                                            | Release                                                                                                                                                                                                    |
| -------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [firmware-inverter](https://github.com/InSol-Tech/firmware-inverter) | [![Build](https://github.com/InSol-Tech/firmware-inverter/actions/workflows/build.yaml/badge.svg)](https://github.com/InSol-Tech/firmware-inverter/actions/workflows/build.yaml) | [![Release](https://github.com/InSol-Tech/firmware-inverter/actions/workflows/build-and-release.yaml/badge.svg)](https://github.com/InSol-Tech/firmware-inverter/actions/workflows/build-and-release.yaml) |
| [firmware-mppt](https://github.com/InSol-Tech/firmware-mppt)         | [![Build](https://github.com/InSol-Tech/firmware-mppt/actions/workflows/build.yaml/badge.svg)](https://github.com/InSol-Tech/firmware-mppt/actions/workflows/build.yaml)         | [![Release](https://github.com/InSol-Tech/firmware-mppt/actions/workflows/build-and-release.yaml/badge.svg)](https://github.com/InSol-Tech/firmware-mppt/actions/workflows/build-and-release.yaml)         |

## Personal Access Tokens

| Name                   | Type         | Scopes                                    | Useage                                  |
| ---------------------- | ------------ | ----------------------------------------- | --------------------------------------- |
| PAT_FIRMWARE_LIBRARIES | Fine-grained | Repository acces to everything firmware-* | Github actions for recursive checkouts. |


## Packages

| Name                                | Description                          | Actions Access    | Useage                                |
| ----------------------------------- | ------------------------------------ | ----------------- | ------------------------------------- |
| firmware-docker-images/stm32cubeclt | Docker image for STM32CubeProgrammer | firmware-* (Read) | Github actions for building firmware. |

## Workflows

| Name                   | Description                             | Container                           | ccu | cm-esp32 | cm-stm32 | dcdc | doc-gen | docker-images      | inverter           | libraries | mppt               |
| ---------------------- | --------------------------------------- | ----------------------------------- | --- | -------- | -------- | ---- | ------- | ------------------ | ------------------ | --------- | ------------------ |
| build.yaml             | Build firmware, used for status checks. | firmware-docker-images/stm32cubeclt |     |          |          |      |         |                    | :white_check_mark: |           | :white_check_mark: |
| build-and-release.yaml | Build and create releases.              | firmware-docker-images/stm32cubeclt |     |          |          |      |         |                    | :white_check_mark: |           | :white_check_mark: |
| stm32cubeclt.yaml      | Build stm32cubeclt package.             |                                     |     |          |          |      |         | :white_check_mark: |                    |           |                    |
