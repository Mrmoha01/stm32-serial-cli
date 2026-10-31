/**
 * @file commands.h
 * @brief The command table: every command the CLI understands.
 */
#ifndef COMMANDS_H
#define COMMANDS_H

#include <stddef.h>

#include "cli.h"

/**
 * @param count receives the number of commands in the table
 * @return the command table
 */
const cli_command_t *commands_get(size_t *count);

#endif /* COMMANDS_H */
