#ifndef FILE_HOOKS_H
#define FILE_HOOKS_H

/*
 * Funções e estruturas para os hooks do sistema de arquivos.
 * O objetivo é interceptar chamadas de acesso a arquivos para esconder
 * a presença de arquivos/diretórios específicos do Wine/Linux.
 */

// Função construtora, chamada quando a biblioteca é carregada.
void file_hooks_init();

#endif // FILE_HOOKS_H
