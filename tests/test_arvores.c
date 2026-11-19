#include "abb.h"
#include "avl.h"
#include "indexador.h"
#include "reader.h"
#include "rubro_negra.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *const BASE_TESTE = "build/base_teste.bin";
static const char *const CSV_TESTE = "build/asteroides_teste.csv";

static CAMPO inteiro(long long valor) {
    CAMPO campo = {.tipo = CAMPO_INTEIRO, .ausente = false};
    campo.valor.inteiro = valor;
    return campo;
}

static void criar_base_identidade(void) {
    FILE *arquivo = fopen(BASE_TESTE, "wb");
    assert(arquivo != NULL);
    for (size_t i = 0; i < 8192; ++i) assert(fputc('x', arquivo) != EOF);
    assert(fclose(arquivo) == 0);
}

static bool arquivos_iguais(const char *a, const char *b) {
    FILE *fa = fopen(a, "rb"), *fb = fopen(b, "rb");
    if (fa == NULL || fb == NULL) {
        if (fa != NULL) fclose(fa);
        if (fb != NULL) fclose(fb);
        return false;
    }
    bool iguais = true;
    int ba, bb;
    do {
        ba = fgetc(fa);
        bb = fgetc(fb);
        if (ba != bb) iguais = false;
    } while (iguais && ba != EOF);
    fclose(fa);
    fclose(fb);
    return iguais;
}

static void arquivo_truncado(const char *caminho) {
    FILE *arquivo = fopen(caminho, "wb");
    assert(arquivo != NULL);
    assert(fwrite("ED2IDX02", 1, 8, arquivo) == 8);
    assert(fclose(arquivo) == 0);
}

#define TESTAR_ARVORE(nome, TIPO, criar, inserir, buscar, tamanho, altura, \
                      salvar, carregar, apagar) \
static void testar_##nome(void) { \
    TIPO *arvore = criar(); \
    assert(arvore != NULL); \
    for (long long i = 1; i <= 40; ++i) { \
        CAMPO chave = inteiro(i); \
        assert(inserir(arvore, &chave, (off_t)(i * 2))); \
    } \
    CAMPO repetida = inteiro(20); \
    assert(inserir(arvore, &repetida, 999)); \
    CAMPO outra_tipo = {.tipo = CAMPO_TEXTO, .ausente = false}; \
    outra_tipo.valor.texto = "20"; \
    assert(!inserir(arvore, &outra_tipo, 1000)); \
    assert(tamanho(arvore) == 40); \
    size_t quantidade = 0; \
    const off_t *offsets = buscar(arvore, &repetida, &quantidade); \
    assert(offsets != NULL && quantidade == 2); \
    assert(offsets[0] == 40 && offsets[1] == 999); \
    CAMPO inexistente = inteiro(100); \
    assert(buscar(arvore, &inexistente, &quantidade) == NULL); \
    assert(quantidade == 0); \
    size_t altura_original = altura(arvore); \
    char indice1[80], indice2[80], truncado[80]; \
    snprintf(indice1, sizeof indice1, "build/%s_1.idx", #nome); \
    snprintf(indice2, sizeof indice2, "build/%s_2.idx", #nome); \
    snprintf(truncado, sizeof truncado, "build/%s_truncado.idx", #nome); \
    assert(salvar(arvore, indice1, "chave", CAMPO_INTEIRO, BASE_TESTE)); \
    apagar(&arvore); \
    arvore = carregar(indice1, "chave", CAMPO_INTEIRO, BASE_TESTE); \
    assert(arvore != NULL); \
    assert(tamanho(arvore) == 40 && altura(arvore) == altura_original); \
    offsets = buscar(arvore, &repetida, &quantidade); \
    assert(offsets != NULL && quantidade == 2); \
    assert(offsets[0] == 40 && offsets[1] == 999); \
    assert(salvar(arvore, indice2, "chave", CAMPO_INTEIRO, BASE_TESTE)); \
    assert(arquivos_iguais(indice1, indice2)); \
    assert(carregar(indice1, "outra_chave", CAMPO_INTEIRO, BASE_TESTE) == NULL); \
    assert(carregar(indice1, "chave", CAMPO_TEXTO, BASE_TESTE) == NULL); \
    arquivo_truncado(truncado); \
    assert(carregar(truncado, "chave", CAMPO_INTEIRO, BASE_TESTE) == NULL); \
    apagar(&arvore); \
}

TESTAR_ARVORE(abb, ABB, abb_criar, abb_inserir, abb_buscar, abb_tamanho,
              abb_altura, abb_salvar, abb_carregar, abb_apagar)
TESTAR_ARVORE(avl, AVL, avl_criar, avl_inserir, avl_buscar, avl_tamanho,
              avl_altura, avl_salvar, avl_carregar, avl_apagar)
TESTAR_ARVORE(rubro_negra, RUBRO_NEGRA, rubro_negra_criar,
              rubro_negra_inserir, rubro_negra_buscar, rubro_negra_tamanho,
              rubro_negra_altura, rubro_negra_salvar, rubro_negra_carregar,
              rubro_negra_apagar)

static bool inserir_avl(void *arvore, const CAMPO *chave, off_t offset) {
    return avl_inserir(arvore, chave, offset);
}

static void escrever_linha(FILE *arquivo, const char *linha) {
    assert(fwrite(linha, 1, strlen(linha), arquivo) == strlen(linha));
}

static void testar_offsets_reader(void) {
    static const char cabecalho[] =
        "asteroid_designation,asteroid_fullname,close_approach_date,year,month,"
        "distance_au,velocity_km_s,absolute_magnitude,days_until_approach,"
        "is_past_event,is_future_event,risk_score,panic_level,threat_category,"
        "panic_verdict,on_sentry_list,sentry_impact_prob,sentry_torino_scale,"
        "sentry_palermo_scale,sentry_diameter_km\r\n";
    static const char linha_a[] =
        "A,A,2024-01-01,2024,1,0.1,1.2,10.0,0,False,True,0.1,1,LOW,OK,"
        "False,0.0,0.0,0.0,0.1\r\n";
    static const char linha_b[] =
        "B,B,2024-02-01,2024,2,0.2,1.3,11.0,1,False,True,0.2,2,LOW,OK,"
        "False,0.0,0.0,0.0,0.2\r\n";

    FILE *arquivo = fopen(CSV_TESTE, "wb");
    assert(arquivo != NULL);
    escrever_linha(arquivo, cabecalho);
    off_t offset_a = ftello(arquivo);
    escrever_linha(arquivo, linha_a);
    off_t offset_b = ftello(arquivo);
    escrever_linha(arquivo, linha_b);
    assert(fclose(arquivo) == 0);

    arquivo = fopen(CSV_TESTE, "rb");
    assert(arquivo != NULL);
    READER *reader = reader_nasa_criar(arquivo);
    assert(reader != NULL);
    size_t indice_ano;
    assert(esquema_buscar(reader_esquema(reader), "year", &indice_ano));
    AVL *arvore = avl_criar();
    assert(arvore != NULL);
    size_t lidos = 0;
    assert(indexador_ler(arquivo, reader, indice_ano, 0, inserir_avl,
                         arvore, &lidos));
    assert(lidos == 2);
    CAMPO ano = inteiro(2024);
    size_t quantidade = 0;
    const off_t *offsets = avl_buscar(arvore, &ano, &quantidade);
    assert(offsets != NULL && quantidade == 2);
    assert(offsets[0] == offset_a && offsets[1] == offset_b);

    assert(fseeko(arquivo, offsets[0], SEEK_SET) == 0);
    assert(fgetc(arquivo) == 'A');
    assert(fseeko(arquivo, offsets[1], SEEK_SET) == 0);
    assert(fgetc(arquivo) == 'B');

    avl_apagar(&arvore);
    reader_apagar(&reader);
    assert(fclose(arquivo) == 0);
}

int main(void) {
    criar_base_identidade();
    testar_abb();
    testar_avl();
    testar_rubro_negra();
    testar_offsets_reader();
    puts("test_arvores: ok");
    return 0;
}
