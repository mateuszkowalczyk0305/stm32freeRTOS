/*
 * commands.h
 *
 *  Created on: May 1, 2026
 *      Author: m_kowalczyk
 */

#ifndef INC_COMMANDS_H_
#define INC_COMMANDS_H_

typedef enum
{
    CMD_ID_START_LED, // id: 0
    CMD_ID_STOP_LED,  // id: 1
    CMD_ID_UNKNOWN    // id: last
} CommandID;

#endif /* INC_COMMANDS_H_ */
