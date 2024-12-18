#include <stdbool.h>
#include "status_register.h"

// Constructor (initialize the register)
void EVERT_SR_Init(EVERT_SR_RegisterTypeDefinitionTypeDef *status_reg)
{
    status_reg->reg = 0;
}

// Get the value of the register
uint32_t EVERT_SR_GetRegister(const EVERT_SR_RegisterTypeDefinitionTypeDef *status_reg)
{
    return status_reg->reg;
}

// Get the count of bits set to 1
uint8_t EVERT_SR_GetBitOnCount(const EVERT_SR_RegisterTypeDefinitionTypeDef *status_reg)
{
    uint8_t count = 0;

    for (uint8_t i = 0; i < sizeof(status_reg->reg) * 8; i++)
    {
        if (status_reg->reg & (1 << i))
        {
            count++;
        }
    }

    return count;
}

// Set a bit at the specified index
bool EVERT_SR_SetBit(EVERT_SR_RegisterTypeDefinitionTypeDef *status_reg, uint8_t index)
{
    if (status_reg->reg & (1 << index))
    {
        return false; // Bit is already set
    }

    status_reg->reg |= (1 << index);

    return true;
}

// Clear a bit at the specified index
bool EVERT_SR_ClearBit(EVERT_SR_RegisterTypeDefinitionTypeDef *status_reg, uint8_t index)
{
    if (!(status_reg->reg & (1 << index)))
    {
        return false; // Bit is not set
    }

    status_reg->reg &= ~(1 << index);

    return true;
}

// Check if a bit at the specified index is set
bool EVERT_SR_IsBitSet(const EVERT_SR_RegisterTypeDefinitionTypeDef *status_reg, uint8_t index)
{
    return (status_reg->reg & (1 << index)) != 0;
}

// Check if any bit in the given array of indexes is set
bool EVERT_SR_IsAnyBitSet(const EVERT_SR_RegisterTypeDefinitionTypeDef *status_reg, const uint8_t *indexes, const uint8_t size)
{
    for (uint8_t i = 0; i < size; i++)
    {
        if (EVERT_SR_IsBitSet(status_reg, indexes[i]))
        {
            return true;
        }
    }
    return false;
}
