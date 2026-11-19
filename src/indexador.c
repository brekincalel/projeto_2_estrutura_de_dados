#include "indexador.h"

#include <stdio.h>

bool indexador_ler(FILE *arquivo, READER *reader, size_t indice_chave,
                   size_t limite, INDEXADOR_INSERIR inserir, void *arvore,
                   size_t *registros_lidos) {
    if (registros_lidos != NULL) *registros_lidos = 0;
    if (arquivo == NULL || reader == NULL || inserir == NULL || arvore == NULL ||
        indice_chave >= esquema_quantidade(reader_esquema(reader)))
        return false;

    size_t lidos = 0;
    while (limite == 0 || lidos < limite) {
        off_t offset = ftello(arquivo);
        if (offset < 0) return false;

        ITEM *item = NULL;
        READER_STATUS status = reader_ler(reader, &item);
        if (status == READER_FIM) break;
        if (status != READER_OK) return false;

        const CAMPO *chave = item_campo(item, indice_chave);
        bool ok = chave != NULL && inserir(arvore, chave, offset);
        item_apagar(&item);
        if (!ok) return false;
        ++lidos;
    }

    if (registros_lidos != NULL) *registros_lidos = lidos;
    return true;
}
