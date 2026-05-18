#ifndef INC_WS2812B_H_
#define INC_WS2812B_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void WS2812_Init(void);

void WS2812_SetActiveLeds(uint8_t activeLedCount,
                          uint8_t brightness,
                          uint8_t red,
                          uint8_t green,
                          uint8_t blue);

uint8_t WS2812_IsBusy(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_WS2812B_H_ */
