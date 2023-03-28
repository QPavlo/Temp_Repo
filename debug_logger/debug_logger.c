#include <stdio.h>
#include <stdarg.h>
#include <time.h>

void log_message(const char *severity, const char *file, const char *func, int line, const char *fmt, ...) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    va_list args;
    va_start(args, fmt);
    printf("[%04d-%02d-%02d %02d:%02d:%02d] [%-15s] %s:%s:%d: ",
           tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
           severity, file, func, line);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

#define LOG_INFO(...) log_message("INFO", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_WARNING(...) log_message("WARNING", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) log_message("ERROR", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_CRITICAL_ERROR(...) log_message("CRITICAL ERROR", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

int main() {
    char *msg1 = "Something happened";
    char *msg2 = "Value is %d";

    LOG_INFO("%s", msg1);
    LOG_WARNING("%s", msg1);
    LOG_ERROR("%s", msg1);
    LOG_CRITICAL_ERROR("%s", msg1);

    int value = 42;
    LOG_INFO(msg2, value);

    return 0;
}
