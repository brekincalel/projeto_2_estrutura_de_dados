#include "chave.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

bool chave_arvore_valida(const CAMPO *campo) {
    if (campo == NULL) return false;
    switch (campo->tipo) {
    case CAMPO_TEXTO:
        return campo->ausente || campo->valor.texto != NULL;
    case CAMPO_INTEIRO:
    case CAMPO_BOOLEANO:
        return true;
    case CAMPO_REAL:
        return campo->ausente || isfinite(campo->valor.real);
    }
    return false;
}

bool chave_arvore_copiar(CHAVE_ARVORE *destino, const CAMPO *origem) {
    if (destino == NULL || !chave_arvore_valida(origem)) return false;

    *destino = (CHAVE_ARVORE){.campo = *origem};
    if (origem->tipo == CAMPO_TEXTO && !origem->ausente) {
        size_t tamanho = strlen(origem->valor.texto);
        if (tamanho == SIZE_MAX) return false;
        destino->texto = malloc(tamanho + 1);
        if (destino->texto == NULL) return false;
        memcpy(destino->texto, origem->valor.texto, tamanho + 1);
        destino->campo.valor.texto = destino->texto;
    } else if (origem->tipo == CAMPO_TEXTO) {
        destino->campo.valor.texto = NULL;
    }
    return true;
}

int chave_arvore_comparar(const CHAVE_ARVORE *a, const CAMPO *b) {
    if (a->campo.tipo != b->tipo)
        return a->campo.tipo < b->tipo ? -1 : 1;
    /* Ausentes ficam depois dos valores, igual a item_comparar_campo. */
    if (a->campo.ausente != b->ausente)
        return a->campo.ausente ? 1 : -1;
    if (a->campo.ausente) return 0;

    switch (a->campo.tipo) {
    case CAMPO_TEXTO:
        return strcmp(a->campo.valor.texto, b->valor.texto);
    case CAMPO_INTEIRO:
        if (a->campo.valor.inteiro == b->valor.inteiro) return 0;
        return a->campo.valor.inteiro < b->valor.inteiro ? -1 : 1;
    case CAMPO_REAL:
        if (a->campo.valor.real == b->valor.real) return 0;
        return a->campo.valor.real < b->valor.real ? -1 : 1;
    case CAMPO_BOOLEANO:
        if (a->campo.valor.booleano == b->valor.booleano) return 0;
        return a->campo.valor.booleano ? 1 : -1;
    }
    return 0;
}

void chave_arvore_apagar(CHAVE_ARVORE *chave) {
    if (chave == NULL) return;
    free(chave->texto);
    *chave = (CHAVE_ARVORE){0};
}

bool offsets_adicionar(LISTA_OFFSETS *lista, off_t offset) {
    if (lista == NULL || offset < 0) return false;
    if (lista->quantidade == lista->capacidade) {
        size_t limite = SIZE_MAX / sizeof(*lista->dados);
        if (lista->quantidade == limite) return false;
        size_t capacidade = lista->capacidade == 0 ? 4 : lista->capacidade;
        if (lista->capacidade != 0) {
            if (capacidade > limite / 2) capacidade = limite;
            else capacidade *= 2;
        }
        if (capacidade > limite) capacidade = limite;
        off_t *dados = realloc(lista->dados, capacidade * sizeof(*dados));
        if (dados == NULL) return false;
        lista->dados = dados;
        lista->capacidade = capacidade;
    }
    lista->dados[lista->quantidade++] = offset;
    return true;
}

void offsets_apagar(LISTA_OFFSETS *lista) {
    if (lista == NULL) return;
    free(lista->dados);
    *lista = (LISTA_OFFSETS){0};
}
