#include <string.h>
#include <stm32g4xx_hal.h>
#include "can_handler.h"

void EVERT_CAN_Identifier_CreateNew(EVERT_CAN_IdentifierTypeDef *identifier)
{
    identifier->message_id = 0;
    identifier->source_id = 0;
    identifier->target_id = 0;
    identifier->evert_flag = 0xE;
    identifier->priority = CAN_MESSAGE_PRIORITY_NORMAL;
}

void EVERT_CAN_Identifier_SetMessageId(EVERT_CAN_IdentifierTypeDef *identifier, EVERT_CAN_MessageIdTypeDef message_id)
{
    identifier->message_id = message_id;
}

void EVERT_CAN_Identifier_SetSourceId(EVERT_CAN_IdentifierTypeDef *identifier, EVERT_CAN_DeviceIdentifierTypeDef source_id)
{
    identifier->source_id = source_id;
}

void EVERT_CAN_Identifier_SetTargetId(EVERT_CAN_IdentifierTypeDef *identifier, EVERT_CAN_DeviceIdentifierTypeDef target_id)
{
    identifier->target_id = target_id;
}

void EVERT_CAN_Identifier_SetPriority(EVERT_CAN_IdentifierTypeDef *identifier, EVERT_CAN_MessagePriorityTypeDef priority)
{
    identifier->priority = priority;
}

void EVERT_CAN_Identifier_FromUint32(const uint32_t id, EVERT_CAN_IdentifierTypeDef *identifier)
{
    // MESSAGE FORMAT

    // typedef struct
    // {
    //     EVERT_CAN_MessageIdTypeDef message_id : 8;       // 0 - 7
    //     EVERT_CAN_DeviceIdentifierTypeDef source_id : 4; // 8 - 11
    //     EVERT_CAN_DeviceIdentifierTypeDef target_id : 4; // 12 - 15
    //     uint8_t evert_flag : 4;                          // 16 - 19 (evert_flag = 0xE)
    //     EVERT_CAN_MessagePriorityTypeDef priority : 4;   // 20 - 23
    //     uint16_t zero_padding : 8;                       // 24 - 31
    // } EVERT_CAN_IdentifierTypeDef;

    // Set identifier based on the message format
    identifier->message_id = id & 0x0FF;
    identifier->source_id = (id >> 8) & 0x0F;
    identifier->target_id = (id >> 12) & 0x0F;
    identifier->evert_flag = 0xE;
    identifier->priority = (id >> 20) & 0x0F;
    identifier->zero_padding = 0;
}

void EVERT_CAN_Identifier_ToUint32(EVERT_CAN_IdentifierTypeDef *identifier, uint32_t *id)
{
    *id = 0;
    *id = (identifier->message_id & 0xFF)           // Bits 0 - 7
          | ((identifier->source_id & 0x0F) << 8)   // Bits 8 - 11
          | ((identifier->target_id & 0x0F) << 12)  // Bits 12 - 15
          | ((identifier->evert_flag & 0x0F) << 16) // Bits 16 - 19
          | ((identifier->priority & 0x0F) << 20);  // Bits 20 - 23
}

void EVERT_CAN_Frame_CreateNew(EVERT_CAN_FrameTypeDef *frame)
{
    EVERT_CAN_Identifier_CreateNew(&frame->identifier);
    frame->data.length = 0;

    for (uint8_t i = 0; i < 7; i++)
    {
        frame->data.data[i] = 0;
    }
}

void EVERT_CAN_Frame_Copy(EVERT_CAN_FrameTypeDef *frame, EVERT_CAN_FrameTypeDef *frame_copy)
{
    frame_copy->identifier = frame->identifier;
    frame_copy->data.length = frame->data.length;

    for (uint8_t i = 0; i < frame->data.length; i++)
    {
        frame_copy->data.data[i] = frame->data.data[i];
    }
}

__attribute__((used)) void EVERT_CAN_Frame_SetData(EVERT_CAN_FrameTypeDef *frame, const uint8_t length, const uint8_t *message)
{
    // Validate input pointers
    if (frame == NULL || message == NULL)
    {
        return;
    }

    // Limit the length to the maximum allowed
    uint8_t actual_length = (length > 7) ? 7 : length;

    // Set the length field
    frame->data.length = actual_length;

    // Copy the data safely
    memcpy(frame->data.data, message, actual_length);
}

void EVERT_CAN_Frame_SetIdentifier(EVERT_CAN_FrameTypeDef *frame, const EVERT_CAN_IdentifierTypeDef identifier)
{
    frame->identifier = identifier;
}

void EVERT_CAN_FifoBuffer_Init(EVERT_CAN_FifoBufferTypeDef *buffer, EVERT_CAN_FifoBufferItemTypeDef *buffer_items, uint32_t size)
{
    buffer->buffer = buffer_items;
    buffer->head = 0;
    buffer->tail = 0;
    buffer->size = size;
    buffer->count = 0;
}

EVERT_CAN_FifoStatusTypeDef EVERT_CAN_FifoBuffer_Push(EVERT_CAN_FifoBufferTypeDef *buffer, const EVERT_CAN_FrameTypeDef frame)
{
    // Check if full
    if (buffer->count == buffer->size)
    {
        return CAN_FS_FULL;
    }

    buffer->buffer[buffer->head].frame = frame;
    buffer->head = (buffer->head + 1) % buffer->size;
    buffer->count++;

    return CAN_FS_OK;
}

EVERT_CAN_FifoStatusTypeDef EVERT_CAN_FifoBuffer_Pop(EVERT_CAN_FifoBufferTypeDef *buffer, EVERT_CAN_FrameTypeDef *frame)
{
    if (buffer->count == 0)
    {
        return CAN_FS_EMPTY;
    }

    *frame = buffer->buffer[buffer->tail].frame;
    buffer->tail = (buffer->tail + 1) % buffer->size;
    buffer->count--;

    return CAN_FS_OK;
}

HAL_StatusTypeDef EVERT_CAN_Handler_Init(FDCAN_HandleTypeDef *hfdcan, EVERT_CAN_HandlerTypeDef *handler, EVERT_CAN_DeviceIdentifierTypeDef device_id, EVERT_CAN_FifoBufferItemTypeDef *rx_fifo_items, EVERT_CAN_FifoBufferItemTypeDef *tx_fifo_items, uint32_t size)
{
    handler->hfdcan = hfdcan;

    // Extended Identifier
    EVERT_CAN_Identifier_CreateNew(&handler->identifier);
    EVERT_CAN_Identifier_SetSourceId(&handler->identifier, device_id);

    // Ring Buffer for RX/TX
    EVERT_CAN_FifoBuffer_Init(&handler->rx_fifo_buffer, rx_fifo_items, size);
    EVERT_CAN_FifoBuffer_Init(&handler->tx_fifo_buffer, tx_fifo_items, size);

    HAL_StatusTypeDef status = HAL_OK;
    FDCAN_FilterTypeDef sFilterConfig;

    /* Configure Rx filter */
    sFilterConfig.IdType = FDCAN_EXTENDED_ID;
    sFilterConfig.FilterIndex = 0;
    sFilterConfig.FilterType = FDCAN_FILTER_MASK;
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    sFilterConfig.FilterID1 = 0;
    sFilterConfig.FilterID2 = 0x1FFFFFFF;

    status = HAL_FDCAN_ConfigFilter(hfdcan, &sFilterConfig);

    if (status != HAL_OK)
    {
        return status;
    }

    // TODO: Move this to Tool implementation
    status = HAL_FDCAN_ConfigGlobalFilter(hfdcan, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);

    if (status != HAL_OK)
    {
        return status;
    }

    /* Start the FDCAN module */
    status = HAL_FDCAN_Start(hfdcan);

    if (status != HAL_OK)
    {
        return status;
    }

    status = HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);

    if (status != HAL_OK)
    {
        return status;
    }

    // Default Tx Header
    handler->tx_header.TxFrameType = FDCAN_DATA_FRAME;
    handler->tx_header.IdType = FDCAN_STANDARD_ID;
    handler->tx_header.DataLength = FDCAN_DLC_BYTES_8;
    handler->tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    handler->tx_header.BitRateSwitch = FDCAN_BRS_OFF;
    handler->tx_header.FDFormat = FDCAN_CLASSIC_CAN;
    handler->tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    handler->tx_header.MessageMarker = 0;

    return HAL_OK;
}

EVERT_CAN_FifoStatusTypeDef EVERT_CAN_Handler_Receive(EVERT_CAN_HandlerTypeDef *handler, const uint32_t RxFifo0ITs)
{
    // Benchmarked @ 17.65 microseconds

    EVERT_CAN_FifoStatusTypeDef status = CAN_FS_OK;

    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
    {
        // EVERT_HAL_DWT_Start();
        if (HAL_FDCAN_GetRxMessage(handler->hfdcan, FDCAN_RX_FIFO0, &handler->rx_header, handler->rx_data) != HAL_OK)
        {
            EVERT_CAN_OnErrorReceived(handler);
            return CAN_FS_ERROR;
        }

        EVERT_CAN_IdentifierTypeDef identifier;
        EVERT_CAN_Identifier_FromUint32(handler->rx_header.Identifier, &identifier);

        EVERT_CAN_FrameDataTypeDef data;
        data.length = handler->rx_data[0];

        for (uint8_t i = 0; i < data.length; i++)
        {
            data.data[i] = handler->rx_data[i + 1];
        }

        EVERT_CAN_FrameTypeDef frame;
        frame.data = data;
        frame.identifier = identifier;

        status = EVERT_CAN_FifoBuffer_Push(&handler->rx_fifo_buffer, frame);
    }

    return status;
}

EVERT_CAN_FifoStatusTypeDef EVERT_CAN_Handler_Transmit(EVERT_CAN_HandlerTypeDef *handler, const uint8_t length, const uint8_t *message)
{
    // EVERT_CAN_FrameDataTypeDef data;
    EVERT_CAN_FrameTypeDef frame;
    EVERT_CAN_Frame_CreateNew(&frame);
    EVERT_CAN_Frame_SetIdentifier(&frame, handler->identifier);
    EVERT_CAN_Frame_SetData(&frame, length, message);

    EVERT_CAN_FifoStatusTypeDef status = EVERT_CAN_FifoBuffer_Push(&handler->tx_fifo_buffer, frame);
    return 0;
}

EVERT_CAN_ProcessBufferStatusTypeDef EVERT_CAN_Handler_ProcessRxBuffer(EVERT_CAN_HandlerTypeDef *handler)
{
    EVERT_CAN_FrameTypeDef frame;

    if (EVERT_CAN_FifoBuffer_Pop(&handler->rx_fifo_buffer, &frame) == CAN_FS_OK)
    {
        handler->rx_status = CAN_PBS_PROCESSING_RECEIVED_DATA;

        EVERT_CAN_OnMessageReceived(handler, frame);

        return CAN_PBS_PROCESSING_RECEIVED_DATA;
    }

    handler->rx_status = CAN_PBS_IDLE;
    return CAN_PBS_IDLE;
}

EVERT_CAN_ProcessBufferStatusTypeDef EVERT_CAN_Handler_ProcessTxBuffer(EVERT_CAN_HandlerTypeDef *handler)
{
    EVERT_CAN_FrameTypeDef frame;
    EVERT_CAN_Frame_CreateNew(&frame);

    if (EVERT_CAN_FifoBuffer_Pop(&handler->tx_fifo_buffer, &frame) == CAN_FS_OK)
    {
        // Set identifier
        handler->tx_header.Identifier = 0;
        EVERT_CAN_Identifier_ToUint32(&frame.identifier, &handler->tx_header.Identifier);

        // Set data
        handler->tx_data[0] = frame.data.length;

        for (uint8_t i = 0; i < frame.data.length; i++)
        {
            handler->tx_data[i + 1] = frame.data.data[i];
        }

        // Add message to queue
        if (HAL_FDCAN_AddMessageToTxFifoQ(handler->hfdcan, &handler->tx_header, handler->tx_data) != HAL_OK)
        {
            handler->tx_status = CAN_PBS_WAITING_FOR_INTERNAL_BUFFER;
            return CAN_PBS_WAITING_FOR_INTERNAL_BUFFER;
        }

        handler->tx_status = CAN_PBS_OK;
        EVERT_CAN_OnMessageTransmitted(handler, frame);
        return CAN_PBS_OK;
    }

    handler->tx_status = CAN_PBS_IDLE;
    return CAN_PBS_IDLE;
}

__weak void EVERT_CAN_OnErrorReceived(EVERT_CAN_HandlerTypeDef *handler)
{
    if (handler->OnErrorReceived != NULL)
    {
        handler->OnErrorReceived();
    }
}

__weak void EVERT_CAN_OnMessageReceived(EVERT_CAN_HandlerTypeDef *handler, const EVERT_CAN_FrameTypeDef frame)
{
    if (handler->OnMessageReceived != NULL)
    {
        handler->OnMessageReceived(frame);
    }
}

__weak void EVERT_CAN_OnMessageTransmitted(EVERT_CAN_HandlerTypeDef *handler, const EVERT_CAN_FrameTypeDef frame)
{
    if (handler->OnMessageTransmitted != NULL)
    {
        handler->OnMessageTransmitted(frame);
    }
}