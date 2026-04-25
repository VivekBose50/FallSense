#pragma once

#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define LOG_BUFFER_SIZE 128

void log_info(const char *fmt, ...);
void log_error(const char *fmt, ...);