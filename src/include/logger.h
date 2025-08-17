#ifndef LOGGER_H
#define LOGGER_H

void logger_init(const char *log_path);
void logger_log(const char *log_path, const char *msg);

#endif // LOGGER_H
