#ifndef ITEM_H
#define ITEM_H

typedef struct item_ ITEM;

ITEM *item_criar(int chave, void *dado);
void item_apagar(ITEM **item);
int item_get_chave(const ITEM *item);
void *item_get_dado(const ITEM *item);

#endif
