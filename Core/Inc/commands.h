#ifndef INC_COMMANDS_H_
#define INC_COMMANDS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t CommandID;

#define CMD_ID_START_LED       ((CommandID)0)
#define CMD_ID_STOP_LED        ((CommandID)1)
#define CMD_ID_SET_LED         ((CommandID)2)
#define CMD_ID_SET_LED_COUNT   ((CommandID)3)
#define CMD_ID_SET_DARK_REF    ((CommandID)4)
#define CMD_ID_SET_LIGHT_REF   ((CommandID)5)
#define CMD_ID_UNKNOWN         ((CommandID)255)

/*
 * UART ustawia tylko kolor RGB:
 * - red
 * - green
 * - blue
 *
 * Liczba aktywnych LED pochodzi z PhotoTask.
 */
typedef struct
{
    uint8_t unused;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} LedCommand;

/*
 * Ta struktura ma mieć dokładnie 12 bajtów,
 * bo QueueLedCommands w .ioc ma Item Size = 12.
 */
typedef struct
{
    CommandID id;          // 1 bajt
    uint8_t ledCount;      // 1 bajt, z PhotoTask
    uint16_t adcRaw;       // 2 bajty, z PhotoTask

    LedCommand led;        // 4 bajty, z UART

    uint8_t reserved[4];   // dopełnienie do 12 bajtów
} CommandMessage;

#ifdef __cplusplus
}
#endif

#endif /* INC_COMMANDS_H_ */
