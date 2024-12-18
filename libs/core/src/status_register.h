#ifndef EVERT_HAL_STATUS_REGISTER_H_
#define EVERT_HAL_STATUS_REGISTER_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint32_t reg; // Equivalent of "register_" in the C++ code
} EVERT_SR_RegisterTypeDefinitionTypeDef;

// TODO: Maybe make the types more explicitly named, but use defines to rename them? Confusion?
#define StatusRegister EVERT_SR_RegisterTypeDefinitionTypeDef
#define SR_TypeDefinition EVERT_SR_RegisterTypeDefinitionTypeDef
#define E_SR EVERT_SR_RegisterTypeDefinitionTypeDef

void EVERT_SR_Init(EVERT_SR_RegisterTypeDefinitionTypeDef *status_reg);
uint32_t EVERT_SR_GetRegister(const EVERT_SR_RegisterTypeDefinitionTypeDef *status_reg);
uint8_t EVERT_SR_GetBitOnCount(const EVERT_SR_RegisterTypeDefinitionTypeDef *status_reg);
bool EVERT_SR_SetBit(EVERT_SR_RegisterTypeDefinitionTypeDef *status_reg, uint8_t index);
bool EVERT_SR_ClearBit(EVERT_SR_RegisterTypeDefinitionTypeDef *status_reg, uint8_t index);
bool EVERT_SR_IsBitSet(const EVERT_SR_RegisterTypeDefinitionTypeDef *status_reg, uint8_t index);
bool EVERT_SR_IsAnyBitSet(const EVERT_SR_RegisterTypeDefinitionTypeDef *status_reg, const uint8_t *indexes, const uint8_t size);

#endif // EVERT_HAL_STATUS_REGISTER_H_