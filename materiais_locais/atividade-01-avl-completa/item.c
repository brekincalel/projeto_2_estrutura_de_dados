#include "item.h"
#include <stdlib.h>

struct item_ {
    int chave;
    void *dado;
};

ITEM *item_criar(int chave, void *dado) {
    ITEM *item = (ITEM *) malloc(sizeof(ITEM));

    if (item != NULL) {
        item->chave = chave;
        item->dado = dado;
    }

    return item;
}

void item_apagar(ITEM **item) {
    if (item == NULL || *item == NULL)
        return;

    free(*item);
    *item = NULL;
}

int item_get_chave(const ITEM *item) {
    if (item == NULL)
        return 0;

    return item->chave;
}

void *item_get_dado(const ITEM *item) {
    if (item == NULL)
        return NULL;

    return item->dado;
}
