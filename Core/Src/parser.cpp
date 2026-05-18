/*
 * parser.cpp
 *
 *  Created on: May 1, 2026
 *      Author: m_kowalczyk
 */

#include "parser.h"
#include "commands.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static uint8_t parseUint8(const char* text, uint8_t* value)
{
    char* endPtr = nullptr;
    long number = strtol(text, &endPtr, 10);

    if (*endPtr != '\0')
    {
        return 0;
    }

    if (number < 0 || number > 255)
    {
        return 0;
    }

    *value = static_cast<uint8_t>(number);
    return 1;
}

CommandMessage parseCommand(const char* cmd)
{
    CommandMessage msg;

    msg.id = CMD_ID_UNKNOWN;
    msg.led.ledNumber = 0;
    msg.led.brightness = 0;
    msg.led.red = 0;
    msg.led.green = 0;
    msg.led.blue = 0;

    if (strcmp(cmd, "START LED") == 0)
    {
        msg.id = CMD_ID_START_LED;
        return msg;
    }

    if (strcmp(cmd, "STOP LED") == 0)
    {
        msg.id = CMD_ID_STOP_LED;
        return msg;
    }

    /*
     * Format:
     * L,<numer_diody>,<jasnosc>,<red>,<green>,<blue>;
     *
     * Przyklad:
     * L,1,255,255,0,0;
     */
    if (cmd[0] == 'L' && cmd[1] == ',')
    {
        char buffer[32];

        strncpy(buffer, cmd, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';

        size_t len = strlen(buffer);

        if (len == 0)
        {
            return msg;
        }

        if (buffer[len - 1] != ';')
        {
            return msg;
        }

        // Usuwamy srednik z konca.
        buffer[len - 1] = '\0';

        char* token = strtok(buffer, ",");

        if (token == nullptr || strcmp(token, "L") != 0)
        {
            return msg;
        }

        uint8_t values[5];

        for (uint8_t i = 0; i < 5; i++)
        {
            token = strtok(nullptr, ",");

            if (token == nullptr)
            {
                return msg;
            }

            if (parseUint8(token, &values[i]) == 0)
            {
                return msg;
            }
        }

        // Jeśli po 5 argumentach jest jeszcze coś, to błąd składni.
        token = strtok(nullptr, ",");

        if (token != nullptr)
        {
            return msg;
        }

        // numer diody: 1-13
        if (values[0] < 1 || values[0] > 13)
        {
            return msg;
        }

        msg.id = CMD_ID_SET_LED;
        msg.led.ledNumber = values[0];
        msg.led.brightness = values[1];
        msg.led.red = values[2];
        msg.led.green = values[3];
        msg.led.blue = values[4];

        return msg;
    }

    return msg;
}
