//
// EMC2303/5 Fan Controller
// https://www.microchip.com/en-us/product/EMC2303#document-table
//

#include "emc230x.h"

static EVERT_EMC230X_RxAction emc2303_rx_action = EVERT_EMC230X_RX_ACTION_NONE;
static EVERT_EMC230X_TxAction emc2303_tx_action = EVERT_EMC230X_TX_ACTION_NONE;

static EVERT_EMC230X_DataTypeDef emc230x_fan1;
static EVERT_EMC230X_DataTypeDef emc230x_fan2;
static EVERT_EMC230X_DataTypeDef emc230x_fan3;

#ifdef EMC230X_TYPE_2305
static EVERT_EMC230X_DataTypeDef emc230x_fan4;
static EVERT_EMC230X_DataTypeDef emc230x_fan5;
#endif

static uint8_t emc2303_range_register = 0;
static uint8_t emc2303_range_register_m = 0;

static inline uint16_t EVERT_EMC230X_CalculateRpm(uint16_t tach)
{
    // https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ApplicationNotes/ApplicationNotes/en562764.pdf
    return (uint16_t)((5.0f - 1.0f) / (2.0f)) * (1.0f / (tach * (1.0f / emc2303_range_register_m))) * (32768.0f) * (60.0f);
}

void EVERT_EMC230X_Init()
{
    EVERT_EMC230X_SetRxAction(EVERT_EMC230X_RX_ACTION_NONE);
    EVERT_EMC230X_SetTxAction(EVERT_EMC230X_TX_ACTION_NONE);

    // TODO: Correct range register
    EVERT_EMC230X_SetRangeRegister_IT(EMC2303_REG_CONFIG_RANGE_1000);
}

void EVERT_EMC230X_SetAllFanSpeeds_IT(uint8_t speed)
{
    if (hi2c1.State != HAL_I2C_STATE_READY)
    {
        return;
    }

    emc230x_fan1.speed = speed;
    emc230x_fan2.speed = speed;
    emc230x_fan3.speed = speed;

#ifdef EMC230X_TYPE_2305
    emc230x_fan4.speed = speed;
    emc230x_fan5.speed = speed;
#endif

    EVERT_EMC230X_SetTxAction(EVERT_EMC230X_TX_ACTION_SET_ALL_FAN1_SPEED);
    EVERT_EMC230X_SetFanSpeed_IT(EVERT_EMC230X_FAN1, speed);
}

void EVERT_EMC230X_SetFanSpeed_IT(const EVERT_EMC230X_Fan fan, uint8_t speed)
{
    if (hi2c1.State != HAL_I2C_STATE_READY)
    {
        return;
    }

    uint8_t emc2303_register = 0;
    uint8_t data[1] = {speed};

    switch (fan)
    {
    case EVERT_EMC230X_FAN1:
        emc2303_register = EMC230X_REG_FAN1_SETTING;
        emc230x_fan1.speed = speed;
        break;

    case EVERT_EMC230X_FAN2:
        emc2303_register = EMC230X_REG_FAN2_SETTING;
        emc230x_fan2.speed = speed;
        break;

    case EVERT_EMC230X_FAN3:
        emc2303_register = EMC230X_REG_FAN3_SETTING;
        emc230x_fan3.speed = speed;
        break;

#ifdef EMC230X_TYPE_2305
    case EVERT_EMC230X_FAN4:
        emc2303_register = EMC230X_REG_FAN4_SETTING;
        emc230x_fan4.speed = speed;
        break;

    case EVERT_EMC230X_FAN5:
        emc2303_register = EMC230X_REG_FAN5_SETTING;
        emc230x_fan5.speed = speed;
        break;
#endif

    default:
        return;
    }

    if (emc2303_tx_action == EVERT_EMC230X_TX_ACTION_NONE)
    {
        EVERT_EMC230X_SetTxAction(EVERT_EMC230X_TX_ACTION_SET_SINGLE_FAN_SPEED);
    }

    if (HAL_I2C_Mem_Write_IT(&hi2c1, EMC230X_ADDRESS << 1, emc2303_register, I2C_MEMADD_SIZE_8BIT, data, 1) != HAL_OK)
    {
        EVERT_EMC230X_OnErrorReceived();
    }
}

void EVERT_EMC230X_SetRangeRegister_IT(const uint8_t range)
{
    emc2303_range_register = range;

    if (range == EMC2303_REG_CONFIG_RANGE_500)
    {
        emc2303_range_register_m = EMC2303_REG_CONFIG_RANGE_500_M;
    }
    else if (range == EMC2303_REG_CONFIG_RANGE_1000)
    {
        emc2303_range_register_m = EMC2303_REG_CONFIG_RANGE_1000_M;
    }
    else if (range == EMC2303_REG_CONFIG_RANGE_2000)
    {
        emc2303_range_register_m = EMC2303_REG_CONFIG_RANGE_2000_M;
    }
    else if (range == EMC2303_REG_CONFIG_RANGE_4000)
    {
        emc2303_range_register_m = EMC2303_REG_CONFIG_RANGE_4000_M;
    }

    uint8_t data[1] = {range};

    // TODO: HAL_Delay garbage

    HAL_I2C_Mem_Write_IT(&hi2c1, EMC230X_ADDRESS << 1, EMC230X_REG_FAN1_CONFIGURATION1, I2C_MEMADD_SIZE_8BIT, data, 1);
    HAL_Delay(1);
    HAL_I2C_Mem_Write_IT(&hi2c1, EMC230X_ADDRESS << 1, EMC230X_REG_FAN2_CONFIGURATION1, I2C_MEMADD_SIZE_8BIT, data, 1);
    HAL_Delay(1);
    HAL_I2C_Mem_Write_IT(&hi2c1, EMC230X_ADDRESS << 1, EMC230X_REG_FAN3_CONFIGURATION1, I2C_MEMADD_SIZE_8BIT, data, 1);
    HAL_Delay(1);

#ifdef EMC230X_TYPE_2305
    HAL_I2C_Mem_Write_IT(&hi2c1, EMC230X_ADDRESS << 1, EMC230X_REG_FAN4_CONFIGURATION1, I2C_MEMADD_SIZE_8BIT, data, 1);
    HAL_Delay(1);
    HAL_I2C_Mem_Write_IT(&hi2c1, EMC230X_ADDRESS << 1, EMC230X_REG_FAN5_CONFIGURATION1, I2C_MEMADD_SIZE_8BIT, data, 1);
    HAL_Delay(1);
#endif
}

void EVERT_EMC230X_SetRxAction(const EVERT_EMC230X_RxAction action)
{
    emc2303_rx_action = action;
}

void EVERT_EMC230X_SetTxAction(const EVERT_EMC230X_TxAction action)
{
    emc2303_tx_action = action;
}

void EVERT_EMC230X_ReadTachs_IT()
{
    if (hi2c1.State != HAL_I2C_STATE_READY)
    {
        return;
    }

    EVERT_EMC230X_SetRxAction(EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN1_HIGH);
    HAL_I2C_Mem_Read_IT(&hi2c1, EMC230X_ADDRESS << 1, EMC230X_REG_FAN1_TACH_HIGH, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&emc230x_fan1.tach_high, 1);
}

__weak void EVERT_EMC230X_OnErrorReceived()
{
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    UNUSED(hi2c);

    switch (emc2303_rx_action)
    {
    case EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN1_HIGH:
    {
        EVERT_EMC230X_SetRxAction(EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN1_LOW);
        HAL_I2C_Mem_Read_IT(&hi2c1, EMC230X_ADDRESS << 1, EMC230X_REG_FAN1_TACH_LOW, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&emc230x_fan1.tach_low, 1);
        break;
    }

    case EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN1_LOW:
    {
        emc230x_fan1.tach = (emc230x_fan1.tach_high << 8) | emc230x_fan1.tach_low;
        emc230x_fan1.rpm = EVERT_EMC230X_CalculateRpm(emc230x_fan1.tach);
        EVERT_EMC230X_SetRxAction(EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN2_HIGH);
        HAL_I2C_Mem_Read_IT(&hi2c1, EMC230X_ADDRESS << 1, EMC230X_REG_FAN2_TACH_HIGH, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&emc230x_fan2.tach_high, 1);
        break;
    }

    case EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN2_HIGH:
    {
        EVERT_EMC230X_SetRxAction(EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN2_LOW);
        HAL_I2C_Mem_Read_IT(&hi2c1, EMC230X_ADDRESS << 1, EMC230X_REG_FAN2_TACH_LOW, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&emc230x_fan2.tach_low, 1);
        break;
    }

    case EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN2_LOW:
    {
        emc230x_fan2.tach = (emc230x_fan2.tach_high << 8) | emc230x_fan2.tach_low;
        emc230x_fan2.rpm = EVERT_EMC230X_CalculateRpm(emc230x_fan2.tach);
        EVERT_EMC230X_SetRxAction(EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN3_HIGH);
        HAL_I2C_Mem_Read_IT(&hi2c1, EMC230X_ADDRESS << 1, EMC230X_REG_FAN3_TACH_HIGH, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&emc230x_fan3.tach_high, 1);
        break;
    }

    case EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN3_HIGH:
    {
        EVERT_EMC230X_SetRxAction(EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN3_LOW);
        HAL_I2C_Mem_Read_IT(&hi2c1, EMC230X_ADDRESS << 1, EMC230X_REG_FAN3_TACH_LOW, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&emc230x_fan3.tach_low, 1);
        break;
    }

    case EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN3_LOW:
    {
        emc230x_fan3.tach = (emc230x_fan3.tach_high << 8) | emc230x_fan3.tach_low;
        emc230x_fan3.rpm = EVERT_EMC230X_CalculateRpm(emc230x_fan3.tach);
        EVERT_EMC230X_SetRxAction(EVERT_EMC230X_RX_ACTION_NONE);

#ifdef EMC230X_TYPE_2305
        EVERT_EMC230X_SetRxAction(EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN4_HIGH);
        HAL_I2C_Mem_Read_IT(&hi2c1, EMC230X_ADDRESS << 1, EMC230X_REG_FAN4_TACH_HIGH, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&emc230x_fan4.tach_high, 1);
#endif
        break;
    }

#ifdef EMC230X_TYPE_2305
    case EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN4_HIGH:
    {
        EVERT_EMC230X_SetRxAction(EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN4_LOW);
        HAL_I2C_Mem_Read_IT(&hi2c1, EMC230X_ADDRESS << 1, EMC230X_REG_FAN4_TACH_LOW, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&emc230x_fan4.tach_low, 1);
        break;
    }

    case EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN4_LOW:
    {
        emc230x_fan4.tach = (emc230x_fan4.tach_high << 8) | emc230x_fan4.tach_low;
        emc230x_fan4.rpm = EVERT_EMC230X_CalculateRpm(emc230x_fan4.tach);
        EVERT_EMC230X_SetRxAction(EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN5_HIGH);
        HAL_I2C_Mem_Read_IT(&hi2c1, EMC230X_ADDRESS << 1, EMC230X_REG_FAN5_TACH_HIGH, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&emc230x_fan5.tach_high, 1);
        break;
    }

    case EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN5_HIGH:
    {
        EVERT_EMC230X_SetRxAction(EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN5_LOW);
        HAL_I2C_Mem_Read_IT(&hi2c1, EMC230X_ADDRESS << 1, EMC230X_REG_FAN5_TACH_LOW, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&emc230x_fan5.tach_low, 1);
        break;
    }

    case EVERT_EMC230X_RX_ACTION_READ_TACHS_FAN5_LOW:
    {
        emc230x_fan5.tach = (emc230x_fan5.tach_high << 8) | emc230x_fan5.tach_low;
        emc230x_fan5.rpm = EVERT_EMC230X_CalculateRpm(emc230x_fan5.tach);
        EVERT_EMC230X_SetRxAction(EVERT_EMC230X_RX_ACTION_NONE);
    }
#endif

    default:
        break;
    }
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    UNUSED(hi2c);

    switch (emc2303_tx_action)
    {
    case EVERT_EMC230X_TX_ACTION_SET_SINGLE_FAN_SPEED:
        EVERT_EMC230X_SetTxAction(EVERT_EMC230X_TX_ACTION_NONE);
        break;

    case EVERT_EMC230X_TX_ACTION_SET_ALL_FAN1_SPEED:
        EVERT_EMC230X_SetTxAction(EVERT_EMC230X_TX_ACTION_SET_ALL_FAN2_SPEED);
        EVERT_EMC230X_SetFanSpeed_IT(EVERT_EMC230X_FAN2, emc230x_fan2.speed);
        break;

    case EVERT_EMC230X_TX_ACTION_SET_ALL_FAN2_SPEED:
        EVERT_EMC230X_SetTxAction(EVERT_EMC230X_TX_ACTION_SET_ALL_FAN3_SPEED);
        EVERT_EMC230X_SetFanSpeed_IT(EVERT_EMC230X_FAN3, emc230x_fan3.speed);
        break;

    case EVERT_EMC230X_TX_ACTION_SET_ALL_FAN3_SPEED:
        EVERT_EMC230X_SetTxAction(EVERT_EMC230X_TX_ACTION_NONE);
#ifdef EMC230X_TYPE_2305
        EVERT_EMC230X_SetTxAction(EVERT_EMC230X_TX_ACTION_SET_ALL_FAN4_SPEED);
        EVERT_EMC230X_SetFanSpeed_IT(EVERT_EMC230X_FAN4, emc230x_fan4.speed);
#endif
        break;

#ifdef EMC230X_TYPE_2305
    case EVERT_EMC230X_TX_ACTION_SET_ALL_FAN4_SPEED:
        EVERT_EMC230X_SetTxAction(EVERT_EMC230X_TX_ACTION_SET_ALL_FAN5_SPEED);
        EVERT_EMC230X_SetFanSpeed_IT(EVERT_EMC230X_FAN5, emc230x_fan5.speed);
        break;

    case EVERT_EMC230X_TX_ACTION_SET_ALL_FAN5_SPEED:
        EVERT_EMC230X_SetTxAction(EVERT_EMC230X_TX_ACTION_NONE);
        break;
#endif

    default:
        break;
    }
}
