#include "logger.h"

extern UART_HandleTypeDef hcom_uart[];          // from BSP

static void log_write(const char *level, const char *fmt, va_list args) {
	char buffer[LOG_BUFFER_SIZE];
	int len = 0;

	len += snprintf(buffer + len, LOG_BUFFER_SIZE - len, "[%s]", level);
	len += vsnprintf(buffer + len, LOG_BUFFER_SIZE - len, fmt, args);
	len += snprintf(buffer + len, LOG_BUFFER_SIZE - len, "\r\n");

	HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t*) buffer, strlen(buffer),
			HAL_MAX_DELAY);
}

void log_info(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_write("[INFO]", fmt, args);
	va_end(args);
}

void log_error(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_write("[ERROR]", fmt, args);
	va_end(args);
}
