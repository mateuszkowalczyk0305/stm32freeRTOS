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
#define CMD_ID_UNKNOWN         ((CommandID)255)

typedef struct
{
    uint8_t ledNumber;
    uint8_t brightness;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} LedCommand;

typedef struct
{
    CommandID id;          // 1 bajt
    uint8_t ledCount;      // 1 bajt
    uint16_t adcRaw;       // 2 bajty

    LedCommand led;        // 5 bajtów

    uint8_t reserved[3];   // dopełnienie do 12 bajtów
} CommandMessage;

#ifdef __cplusplus
}
#endif

#endif /* INC_COMMANDS_H_ */
