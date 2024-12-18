#ifndef EVERT_CAN_HANDLER_H_
#define EVERT_CAN_HANDLER_H_

#include <stdlib.h>
#include <stdint.h>
#include <stm32g4xx_hal.h>

#pragma pack(push, 1)

typedef enum
{
    CAN_MESSAGE_ID_MIN = 0,
    CAN_MESSAGE_ID_MAX = 255
} EVERT_CAN_MessageIdTypeDef;

typedef enum
{
    CAN_DEVICE_IDENTIFIER_UNIDENTIFIED = 0,
    CAN_DEVICE_IDENTIFIER_BOOST_CONVERTER1 = 1,
    CAN_DEVICE_IDENTIFIER_BOOST_CONVERTER2 = 2,
    CAN_DEVICE_IDENTIFIER_MAX = 127
} EVERT_CAN_DeviceIdentifierTypeDef;

typedef enum
{
    CAN_MESSAGE_PRIORITY_CRITICAL = 0,
    CAN_MESSAGE_PRIORITY_HIGH = 1,
    CAN_MESSAGE_PRIORITY_NORMAL = 2,
    CAN_MESSAGE_PRIORITY_LOW = 3,
    CAN_MESSAGE_PRIORITY_MAX = 127
} EVERT_CAN_MessagePriorityTypeDef;

typedef struct
{
    EVERT_CAN_MessageIdTypeDef message_id : 8;       // 0 - 7
    EVERT_CAN_DeviceIdentifierTypeDef source_id : 4; // 8 - 11
    EVERT_CAN_DeviceIdentifierTypeDef target_id : 4; // 12 - 15
    uint8_t evert_flag : 4;                          // 16 - 19 (evert_flag = 0xE)
    EVERT_CAN_MessagePriorityTypeDef priority : 4;   // 20 - 23
    uint16_t zero_padding : 8;                       // 24 - 31
} EVERT_CAN_IdentifierTypeDef;

#pragma pack(pop)

/** @defgroup EVERT CAN IDENTIFIER
 * @brief EVERT CAN Extended Identifier Type Definition Functions
 * @{
 */
void EVERT_CAN_Identifier_CreateNew(EVERT_CAN_IdentifierTypeDef *identifier);
void EVERT_CAN_Identifier_SetMessageId(EVERT_CAN_IdentifierTypeDef *identifier, EVERT_CAN_MessageIdTypeDef message_id);
void EVERT_CAN_Identifier_SetSourceId(EVERT_CAN_IdentifierTypeDef *identifier, EVERT_CAN_DeviceIdentifierTypeDef source_id);
void EVERT_CAN_Identifier_SetTargetId(EVERT_CAN_IdentifierTypeDef *identifier, EVERT_CAN_DeviceIdentifierTypeDef target_id);
void EVERT_CAN_Identifier_SetPriority(EVERT_CAN_IdentifierTypeDef *identifier, EVERT_CAN_MessagePriorityTypeDef priority);
void EVERT_CAN_Identifier_FromUint32(uint32_t id, EVERT_CAN_IdentifierTypeDef *identifier);
void EVERT_CAN_Identifier_ToUint32(EVERT_CAN_IdentifierTypeDef *identifier, uint32_t *id);
/** @} */

/** @defgroup EVERT CAN FRAME
 * @brief EVERT CAN Frame Data Type Definition Functions
 * @{
 */
typedef struct
{
    uint8_t length;  // Size: 1B
    uint8_t data[7]; // Size: 0-7B
} EVERT_CAN_FrameDataTypeDef;

typedef struct
{
    EVERT_CAN_IdentifierTypeDef identifier;
    EVERT_CAN_FrameDataTypeDef data;
} EVERT_CAN_FrameTypeDef;

void EVERT_CAN_Frame_CreateNew(EVERT_CAN_FrameTypeDef *frame);
void EVERT_CAN_Frame_Copy(EVERT_CAN_FrameTypeDef *frame, EVERT_CAN_FrameTypeDef *frame_copy);
void EVERT_CAN_Frame_SetData(EVERT_CAN_FrameTypeDef *frame, const uint8_t length, const uint8_t *message);
void EVERT_CAN_Frame_SetIdentifier(EVERT_CAN_FrameTypeDef *frame, const EVERT_CAN_IdentifierTypeDef identifier);

/** @} */

/** @defgroup EVERT CAN RING BUFFER
 * @brief
 * @{
 */

typedef enum
{
    CAN_FS_OK = 0,
    CAN_FS_EMPTY = 1,
    CAN_FS_FULL = 2,
    CAN_FS_OVERRUN = 3,
    CAN_FS_ERROR = 4
} EVERT_CAN_FifoStatusTypeDef;

typedef struct
{
    EVERT_CAN_FrameTypeDef frame;
} EVERT_CAN_FifoBufferItemTypeDef;

typedef struct
{
    EVERT_CAN_FifoBufferItemTypeDef *buffer;
    uint32_t head;
    uint32_t tail;
    uint32_t size;
    uint32_t count;
} EVERT_CAN_FifoBufferTypeDef;

void EVERT_CAN_FifoBuffer_Init(EVERT_CAN_FifoBufferTypeDef *buffer, EVERT_CAN_FifoBufferItemTypeDef *buffer_items, uint32_t size);
EVERT_CAN_FifoStatusTypeDef EVERT_CAN_FifoBuffer_Push(EVERT_CAN_FifoBufferTypeDef *buffer, const EVERT_CAN_FrameTypeDef frame);
EVERT_CAN_FifoStatusTypeDef EVERT_CAN_FifoBuffer_Pop(EVERT_CAN_FifoBufferTypeDef *buffer, EVERT_CAN_FrameTypeDef *frame);

/** @} */

/** @defgroup EVERT CAN HANDLER
 * @brief
 * @{
 */

typedef enum
{
    CAN_PBS_OK = 0,
    CAN_PBS_IDLE = 1,
    CAN_PBS_PROCESSING_RECEIVED_DATA = 2,
    CAN_PBS_WAITING_FOR_INTERNAL_BUFFER = 3
} EVERT_CAN_ProcessBufferStatusTypeDef;

typedef struct
{
    void (*OnErrorReceived)(void);
    void (*OnMessageReceived)(const EVERT_CAN_FrameTypeDef frame);
    void (*OnMessageTransmitted)(const EVERT_CAN_FrameTypeDef frame);

    FDCAN_HandleTypeDef *hfdcan;
    EVERT_CAN_IdentifierTypeDef identifier;
    EVERT_CAN_FifoBufferTypeDef rx_fifo_buffer;
    EVERT_CAN_FifoBufferTypeDef tx_fifo_buffer;
    FDCAN_RxHeaderTypeDef rx_header;
    FDCAN_TxHeaderTypeDef tx_header;
    uint8_t rx_data[8];
    uint8_t tx_data[8];
    EVERT_CAN_ProcessBufferStatusTypeDef rx_status;
    EVERT_CAN_ProcessBufferStatusTypeDef tx_status;

} EVERT_CAN_HandlerTypeDef;

HAL_StatusTypeDef EVERT_CAN_Handler_Init(FDCAN_HandleTypeDef *hfdcan, EVERT_CAN_HandlerTypeDef *handler, EVERT_CAN_DeviceIdentifierTypeDef device_id, EVERT_CAN_FifoBufferItemTypeDef *rx_fifo_items, EVERT_CAN_FifoBufferItemTypeDef *tx_fifo_items, uint32_t size);
EVERT_CAN_FifoStatusTypeDef EVERT_CAN_Handler_Receive(EVERT_CAN_HandlerTypeDef *handler, const uint32_t RxFifo0ITs);
EVERT_CAN_FifoStatusTypeDef EVERT_CAN_Handler_Transmit(EVERT_CAN_HandlerTypeDef *handler, const uint8_t length, const uint8_t *message);
EVERT_CAN_ProcessBufferStatusTypeDef EVERT_CAN_Handler_ProcessRxBuffer(EVERT_CAN_HandlerTypeDef *handler);
EVERT_CAN_ProcessBufferStatusTypeDef EVERT_CAN_Handler_ProcessTxBuffer(EVERT_CAN_HandlerTypeDef *handler);

void EVERT_CAN_OnErrorReceived(EVERT_CAN_HandlerTypeDef *handler);
void EVERT_CAN_OnMessageReceived(EVERT_CAN_HandlerTypeDef *handler, const EVERT_CAN_FrameTypeDef frame);
void EVERT_CAN_OnMessageTransmitted(EVERT_CAN_HandlerTypeDef *handler, const EVERT_CAN_FrameTypeDef frame);

#endif // EVERT_CAN_HANDLER_H_