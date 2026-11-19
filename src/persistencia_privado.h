#ifndef PERSISTENCIA_PRIVADO_H
#define PERSISTENCIA_PRIVADO_H

#include "chave.h"

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* Inteiros do arquivo são little-endian, com larguras fixas. */
static bool tipo_campo_valido(TIPO_CAMPO tipo) {
    switch (tipo) {
    case CAMPO_TEXTO:
    case CAMPO_INTEIRO:
    case CAMPO_REAL:
    case CAMPO_BOOLEANO:
        return true;
    }
    return false;
}

static bool gravar_u8(FILE *f, uint8_t valor) {
    return fwrite(&valor, 1, 1, f) == 1;
}

static bool ler_u8(FILE *f, uint8_t *valor) {
    return fread(valor, 1, 1, f) == 1;
}

static bool gravar_u32(FILE *f, uint32_t valor) {
    for (unsigned i = 0; i < 4; ++i)
        if (!gravar_u8(f, (uint8_t)(valor >> (8 * i)))) return false;
    return true;
}

static bool ler_u32(FILE *f, uint32_t *valor) {
    uint32_t resultado = 0;
    for (unsigned i = 0; i < 4; ++i) {
        uint8_t byte;
        if (!ler_u8(f, &byte)) return false;
        resultado |= (uint32_t)byte << (8 * i);
    }
    *valor = resultado;
    return true;
}

static bool gravar_u64(FILE *f, uint64_t valor) {
    for (unsigned i = 0; i < 8; ++i)
        if (!gravar_u8(f, (uint8_t)(valor >> (8 * i)))) return false;
    return true;
}

static bool ler_u64(FILE *f, uint64_t *valor) {
    uint64_t resultado = 0;
    for (unsigned i = 0; i < 8; ++i) {
        uint8_t byte;
        if (!ler_u8(f, &byte)) return false;
        resultado |= (uint64_t)byte << (8 * i);
    }
    *valor = resultado;
    return true;
}

static bool gravar_i64(FILE *f, int64_t valor) {
    return gravar_u64(f, (uint64_t)valor);
}

static bool ler_i64(FILE *f, int64_t *valor) {
    uint64_t bits;
    if (!ler_u64(f, &bits)) return false;
    if (bits <= INT64_MAX) *valor = (int64_t)bits;
    else *valor = -1 - (int64_t)(~bits);
    return true;
}

static bool gravar_texto(FILE *f, const char *texto) {
    size_t tamanho = strlen(texto);
    if (tamanho > 1024 * 1024 || tamanho > UINT32_MAX ||
        !gravar_u32(f, (uint32_t)tamanho)) return false;
    return fwrite(texto, 1, tamanho, f) == tamanho;
}

static bool conferir_texto(FILE *f, const char *esperado) {
    uint32_t tamanho;
    if (!ler_u32(f, &tamanho) || tamanho > 1024 * 1024 ||
        tamanho != strlen(esperado)) return false;
    char *texto = malloc((size_t)tamanho + 1);
    if (texto == NULL) return false;
    bool ok = fread(texto, 1, tamanho, f) == tamanho;
    texto[tamanho] = '\0';
    ok = ok && strcmp(texto, esperado) == 0;
    free(texto);
    return ok;
}

/* Identidade prática da base; não relê o CSV nem calcula hash do conteúdo. */
static bool persist_cabecalho(FILE *f, const char *arvore, const char *coluna,
                              TIPO_CAMPO tipo_chave, const char *dados,
                              bool escrever) {
    static const char assinatura[8] = {'E','D','2','I','D','X','0','2'};
    struct stat st;
    if (f == NULL || arvore == NULL || coluna == NULL || dados == NULL ||
        !tipo_campo_valido(tipo_chave) || stat(dados, &st) != 0 ||
        st.st_size < 0) return false;
    uint64_t tamanho = (uint64_t)st.st_size;
    int64_t modificacao = (int64_t)st.st_mtime;
    int64_t mudanca = (int64_t)st.st_ctime;

    if (escrever) {
        return fwrite(assinatura, 1, sizeof assinatura, f) == sizeof assinatura &&
               gravar_u32(f, 2) && gravar_texto(f, arvore) &&
               gravar_texto(f, coluna) &&
               gravar_u8(f, (uint8_t)tipo_chave) && gravar_texto(f, dados) &&
               gravar_u64(f, tamanho) && gravar_i64(f, modificacao) &&
               gravar_i64(f, mudanca);
    }

    char lida[sizeof assinatura];
    uint32_t versao;
    uint8_t tipo_lido;
    uint64_t tamanho_lido;
    int64_t modificacao_lida, mudanca_lida;
    return fread(lida, 1, sizeof lida, f) == sizeof lida &&
           memcmp(lida, assinatura, sizeof assinatura) == 0 &&
           ler_u32(f, &versao) && versao == 2 &&
           conferir_texto(f, arvore) && conferir_texto(f, coluna) &&
           ler_u8(f, &tipo_lido) && tipo_lido == (uint8_t)tipo_chave &&
           conferir_texto(f, dados) && ler_u64(f, &tamanho_lido) &&
           ler_i64(f, &modificacao_lida) && ler_i64(f, &mudanca_lida) &&
           tamanho_lido == tamanho && modificacao_lida == modificacao &&
           mudanca_lida == mudanca;
}

static bool gravar_chave(FILE *f, const CHAVE_ARVORE *chave) {
    const CAMPO *campo = &chave->campo;
    if (!gravar_u8(f, (uint8_t)campo->tipo) ||
        !gravar_u8(f, (uint8_t)campo->ausente)) return false;
    if (campo->ausente) return true;

    switch (campo->tipo) {
    case CAMPO_TEXTO:
        return gravar_u64(f, (uint64_t)strlen(campo->valor.texto)) &&
               fwrite(campo->valor.texto, 1, strlen(campo->valor.texto), f) ==
                   strlen(campo->valor.texto);
    case CAMPO_INTEIRO: {
        int64_t valor = (int64_t)campo->valor.inteiro;
        return (long long)valor == campo->valor.inteiro && gravar_i64(f, valor);
    }
    case CAMPO_REAL: {
        if (sizeof(double) != sizeof(uint64_t) || DBL_MANT_DIG != 53 ||
            DBL_MAX_EXP != 1024 || !isfinite(campo->valor.real)) return false;
        uint64_t bits;
        memcpy(&bits, &campo->valor.real, sizeof bits);
        return gravar_u64(f, bits);
    }
    case CAMPO_BOOLEANO:
        return gravar_u8(f, campo->valor.booleano ? 1 : 0);
    }
    return false;
}

static bool ler_chave(FILE *f, CHAVE_ARVORE *chave, TIPO_CAMPO tipo_esperado) {
    uint8_t tipo, ausente;
    if (!ler_u8(f, &tipo) || !ler_u8(f, &ausente) ||
        tipo > CAMPO_BOOLEANO || tipo != tipo_esperado || ausente > 1) return false;
    CAMPO campo = {.tipo = (TIPO_CAMPO)tipo, .ausente = ausente != 0};
    if (!campo.ausente) {
        switch (campo.tipo) {
        case CAMPO_TEXTO: {
            uint64_t tamanho;
            if (!ler_u64(f, &tamanho) || tamanho > SIZE_MAX - 1) return false;
            char *texto = malloc((size_t)tamanho + 1);
            if (texto == NULL) return false;
            bool ok = fread(texto, 1, (size_t)tamanho, f) == tamanho;
            if (ok && memchr(texto, '\0', (size_t)tamanho) != NULL) ok = false;
            texto[tamanho] = '\0';
            campo.valor.texto = texto;
            ok = ok && chave_arvore_copiar(chave, &campo);
            free(texto);
            return ok;
        }
        case CAMPO_INTEIRO: {
            int64_t valor;
            if (!ler_i64(f, &valor)) return false;
            campo.valor.inteiro = (long long)valor;
            if ((int64_t)campo.valor.inteiro != valor) return false;
            break;
        }
        case CAMPO_REAL: {
            uint64_t bits;
            if (sizeof(double) != sizeof bits || DBL_MANT_DIG != 53 ||
                DBL_MAX_EXP != 1024 || !ler_u64(f, &bits)) return false;
            memcpy(&campo.valor.real, &bits, sizeof bits);
            break;
        }
        case CAMPO_BOOLEANO: {
            uint8_t valor;
            if (!ler_u8(f, &valor) || valor > 1) return false;
            campo.valor.booleano = valor != 0;
            break;
        }
        }
    }
    return chave_arvore_copiar(chave, &campo);
}

static bool gravar_offsets(FILE *f, const LISTA_OFFSETS *offsets) {
    if (!gravar_u64(f, (uint64_t)offsets->quantidade)) return false;
    for (size_t i = 0; i < offsets->quantidade; ++i) {
        if (offsets->dados[i] < 0 ||
            (off_t)(uint64_t)offsets->dados[i] != offsets->dados[i] ||
            !gravar_u64(f, (uint64_t)offsets->dados[i])) return false;
    }
    return offsets->quantidade != 0;
}

static bool ler_offsets(FILE *f, LISTA_OFFSETS *offsets) {
    uint64_t quantidade;
    if (!ler_u64(f, &quantidade) || quantidade == 0 ||
        quantidade > SIZE_MAX / sizeof(off_t)) return false;
    for (uint64_t i = 0; i < quantidade; ++i) {
        uint64_t valor;
        if (!ler_u64(f, &valor)) return false;
        off_t offset = (off_t)valor;
        if (offset < 0 || (uint64_t)offset != valor ||
            !offsets_adicionar(offsets, offset)) return false;
    }
    return true;
}

static bool offsets_no_arquivo(const LISTA_OFFSETS *offsets,
                               uint64_t tamanho_arquivo) {
    if (offsets->quantidade == 0) return false;
    for (size_t i = 0; i < offsets->quantidade; ++i)
        if (offsets->dados[i] < 0 ||
            (uint64_t)offsets->dados[i] >= tamanho_arquivo) return false;
    return true;
}

static bool persist_tamanho_arquivo(const char *caminho, uint64_t *tamanho) {
    struct stat st;
    if (caminho == NULL || tamanho == NULL || stat(caminho, &st) != 0 ||
        st.st_size < 0) return false;
    *tamanho = (uint64_t)st.st_size;
    return true;
}

static bool persist_caminhos_distintos(const char *indice, const char *dados) {
    if (indice == NULL || dados == NULL || strcmp(indice, dados) == 0)
        return false;
    struct stat st_indice, st_dados;
    return stat(dados, &st_dados) == 0 &&
           (stat(indice, &st_indice) != 0 ||
            st_indice.st_dev != st_dados.st_dev ||
            st_indice.st_ino != st_dados.st_ino);
}

#endif
