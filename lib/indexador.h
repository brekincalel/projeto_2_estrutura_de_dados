#ifndef INDEXADOR_H
#define INDEXADOR_H

#include "reader.h"

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

typedef bool (*INDEXADOR_INSERIR)(void *arvore, const CAMPO *chave,
                                  off_t offset);

/* O reader já deve ter sido criado (e, portanto, já consumiu o cabeçalho).
 * limite zero significa ler até EOF. O callback deve copiar a chave se
 * precisar mantê-la após retornar. */
bool indexador_ler(FILE *arquivo, READER *reader, size_t indice_chave,
                   size_t limite, INDEXADOR_INSERIR inserir, void *arvore,
                   size_t *registros_lidos);

#endif
