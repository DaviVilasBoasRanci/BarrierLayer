#ifndef PATH_UTILS_H
#define PATH_UTILS_H

// Retorna o caminho absoluto para o arquivo de log principal.
// O caminho é construído dinamicamente usando o diretório HOME do usuário.
const char* get_log_path(void);

#endif // PATH_UTILS_H
