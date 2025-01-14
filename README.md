# GitOps

## Quick References

[Github/Evert/Firmware-*](https://github.com/orgs/InSol-Tech/repositories?language=&q=firmware-&sort=&type=all)

## Status


[![Release](https://github.com/InSol-Tech/firmware-ccu/actions/workflows/build-and-release.yaml/badge.svg)](https://github.com/InSol-Tech/firmware-ccu/actions/workflows/build-and-release.yaml)

| Repository                                                           | Build                                                                                                                                                                            | Release                                                                                                                                                                                                    |
| -------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [firmware-ccu](https://github.com/InSol-Tech/firmware-ccu)           | [![Build](https://github.com/InSol-Tech/firmware-ccu/actions/workflows/build.yaml/badge.svg)](https://github.com/InSol-Tech/firmware-ccu/actions/workflows/build.yaml)           | [![Release](https://github.com/InSol-Tech/firmware-ccu/actions/workflows/build-and-release.yaml/badge.svg)](https://github.com/InSol-Tech/firmware-ccu/actions/workflows/build-and-release.yaml)           |
| [firmware-dcdc](https://github.com/InSol-Tech/firmware-dcdc)         | [![Build](https://github.com/InSol-Tech/firmware-dcdc/actions/workflows/build.yaml/badge.svg)](https://github.com/InSol-Tech/firmware-dcdc/actions/workflows/build.yaml)         | [![Release](https://github.com/InSol-Tech/firmware-dcdc/actions/workflows/build-and-release.yaml/badge.svg)](https://github.com/InSol-Tech/firmware-dcdc/actions/workflows/build-and-release.yaml)         |
| [firmware-inverter](https://github.com/InSol-Tech/firmware-inverter) | [![Build](https://github.com/InSol-Tech/firmware-inverter/actions/workflows/build.yaml/badge.svg)](https://github.com/InSol-Tech/firmware-inverter/actions/workflows/build.yaml) | [![Release](https://github.com/InSol-Tech/firmware-inverter/actions/workflows/build-and-release.yaml/badge.svg)](https://github.com/InSol-Tech/firmware-inverter/actions/workflows/build-and-release.yaml) |
| [firmware-mppt](https://github.com/InSol-Tech/firmware-mppt)         | [![Build](https://github.com/InSol-Tech/firmware-mppt/actions/workflows/build.yaml/badge.svg)](https://github.com/InSol-Tech/firmware-mppt/actions/workflows/build.yaml)         | [![Release](https://github.com/InSol-Tech/firmware-mppt/actions/workflows/build-and-release.yaml/badge.svg)](https://github.com/InSol-Tech/firmware-mppt/actions/workflows/build-and-release.yaml)         |

## Personal Access Tokens

| Name                   | Type         | Scopes                                     | Useage                                  |
| ---------------------- | ------------ | ------------------------------------------ | --------------------------------------- |
| PAT_FIRMWARE_LIBRARIES | Fine-grained | Repository access to everything firmware-* | Github actions for recursive checkouts. |


## Packages

| Name                                | Description                          | Actions Access    | Useage                                |
| ----------------------------------- | ------------------------------------ | ----------------- | ------------------------------------- |
| firmware-docker-images/stm32cubeclt | Docker image for STM32CubeProgrammer | firmware-* (Read) | Github actions for building firmware. |

## Workflows

### Descriptions

| Name                           | Description                             | Initiator            | Container                           |
| ------------------------------ | --------------------------------------- | -------------------- | ----------------------------------- |
| build.yaml (STM32)             | Build firmware, used for status checks. | PR -> main           | firmware-docker-images/stm32cubeclt |
| build-and-release.yaml (STM32) | Build and create releases.              | Tag: v*              | firmware-docker-images/stm32cubeclt |
| stm32cubeclt.yaml              | Build stm32cubeclt package.             | Tag: stm32cubeclt-v* |                                     |

### Usage

| Name                           | ccu                | cm-esp32 | cm-stm32 | dcdc               | doc-gen | docker-images      | inverter           | libraries | mppt               |
| ------------------------------ | ------------------ | -------- | -------- | ------------------ | ------- | ------------------ | ------------------ | --------- | ------------------ |
| build.yaml (STM32)             | :white_check_mark: |          |          | :white_check_mark: |         |                    | :white_check_mark: | :one:     | :white_check_mark: |
| build-and-release.yaml (STM32) | :white_check_mark: |          |          | :white_check_mark: |         |                    | :white_check_mark: | :one:     | :white_check_mark: |
| stm32cubeclt.yaml              |                    |          |          |                    |         | :white_check_mark: |                    |           |                    |

:one: Libraries are built as part of the firmware build (sub-module).
