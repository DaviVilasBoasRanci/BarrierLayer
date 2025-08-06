#ifndef LOGGER_H
#define LOGGER_H

void logger_init(const char *log_path);
void logger_log(const char *log_path, const char *msg);

#endif // LOGGER_H
#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>

// Inicializa o sistema de log (abre o arquivo)
void log_init(const char* filename);

// Registra uma mensagem formatada no log
void log_message(const char* format, ...);

// Fecha o sistema de log (fecha o arquivo)
void log_close();

#endif // LOGGER_H
