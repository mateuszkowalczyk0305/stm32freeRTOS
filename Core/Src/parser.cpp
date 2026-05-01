/*
 * parser.cpp
 *
 *  Created on: May 1, 2026
 *      Author: m_kowalczyk
 */

#include "commands.h"
#include <string.h>

CommandID parseCommand(const char* cmd)
{
    if (strcmp(cmd, "START LED") == 0)
        return CMD_ID_START_LED;

    if (strcmp(cmd, "STOP LED") == 0)
        return CMD_ID_STOP_LED;

    return CMD_ID_UNKNOWN;
}


