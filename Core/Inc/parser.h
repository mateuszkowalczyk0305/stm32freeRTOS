/*
 * parser.h
 *
 *  Created on: May 1, 2026
 *      Author: m_kowalczyk
 */

#pragma once

#ifndef INC_PARSER_H_
#define INC_PARSER_H_

#include "commands.h"

CommandID parseCommand(const char* cmd);

#endif /* INC_PARSER_H_ */
