#ifdef DEBUG_PRINT
#define printf_debug(format, ...) \
do { \
if (*#format) { \
fprintf(stderr, "\033[38;5;214m%s:%-4d | %15s | " format "\033[0m\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
} else { \
fprintf(stderr, "\033[38;5;214m%s:%-4d | %15s | \033[0m\n", __FILE__, __LINE__, __func__); \
} \
fflush(stderr); \
} while (0)
#else
#define printf_debug(format, ...) (0)
#endif
