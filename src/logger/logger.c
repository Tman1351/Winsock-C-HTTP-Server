#include "logger.h"

// TODO: temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#define MSG_LENGTH 32000


void log_output(log_level level, const char* message, ...) {
    const char* level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
    b8 is_error = level < LOG_LEVEL_WARN;

    char out_message[MSG_LENGTH];
    memset(out_message, 0, sizeof(out_message));

    // Format original message
    va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(out_message, MSG_LENGTH, message, arg_ptr);
    va_end(arg_ptr);
    
    char out_message2[MSG_LENGTH];
    // Append output type at start of output
    snprintf(out_message2, MSG_LENGTH, "%s%s\n", level_strings[level], out_message);

    if (is_error) {
        fprintf(stderr, "%s", out_message2);
    } else {
        fprintf(stdout, "%s", out_message2);
    }
}
