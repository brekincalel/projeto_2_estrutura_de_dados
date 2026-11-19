# Projeto 2 — Árvores de busca e índices em disco

SCC0202 — Algoritmos e Estruturas de Dados I. Implementação em C de índices com ABB, AVL e rubro-negra sobre os datasets de asteroides e Mapa de Empresas do Projeto 1.

Este documento é um **plano de ação**, baseado no [enunciado](materiais_locais/projeto2.pdf), nos materiais do professor e no código atual. As etapas desmarcadas ainda têm implementação ou critérios de aceite pendentes. Os comandos de execução apresentados são exemplos-alvo, não funcionalidades já disponíveis.

## Diagnóstico inicial

Revisão de 19/11/2026. Referência anterior: [Projeto 1](https://github.com/LuizCunha4/projeto_1_estrutura_de_dados), commit `98f4cd56088971ccda40d6262fddc1be7861ece5`.

| Componente | Estado observado | Próxima ação |
| --- | --- | --- |
| [CSV](src/csv.c), [ITEM](src/item.c), [reader](src/reader.c), [NASA](src/reader_nasa.c), [empresas](src/reader_empresas.c) e [header privado](src/reader_privado.h) | Os seis arquivos são idênticos aos do Projeto 1. Já leem um registro por chamada e preservam o texto original. | Reaproveitar parsing, acrescentar offsets e leitura direta; revisar contratos herdados da base em memória. |
| [Chave](lib/chave.h) e [implementação](src/chave.c) | Já existem chave tipada, cópia de textos, lista dinâmica de offsets e métricas compartilhadas. | Include e guarda foram corrigidos; ausentes ordenam por último e árvores rejeitam mistura de tipos. Compilar e testar ainda está pendente. |
| [ABB](src/abb.c), [AVL](src/avl.c) e [rubro-negra](src/rubro_negra.c) | Inserção, busca, altura, liberação e métricas. | Implementadas APIs locais de persistência. Rubro-negra foi convertida para LLRB. Invariantes ainda precisam de compilação e testes funcionais. |
| [ITEM atual](lib/item.h) | Representa a linha completa, sem offset, e mantém contratos BASE/RESULTADOS/ORDER do trabalho anterior. | Separar registro temporário e item compacto do índice. |
| Aplicação | Ainda não há programa principal nem consultas. Há build dos módulos, testes básicos e helper de indexação sequencial. | Criar CLI, ligar seleção das árvores ao indexador e implementar busca por offset. |
| Materiais locais | [.gitignore](.gitignore) vazio; vários arquivos de [materiais_locais/](materiais_locais/) já são rastreados, inclusive objetos e executável de exemplo. | Combinar política de versionamento. A afirmação antiga de que a pasta estaria ignorada não corresponde ao estado atual. |

Verificação executada: `gcc -std=c11 -Wall -Wextra -Wpedantic -Ilib -fsyntax-only src/*.c`. Falhou no include inexistente de [src/chave.c](src/chave.c). Isso é diagnóstico de compilação, não teste funcional.

## Atualização após o fetch e decisões implementadas

O commit `de2a5fc` foi integrado em fast-forward. Os dois datasets agora estão
em `data/`; o [makefile](makefile) continua vazio. A tabela acima registra o
estado anterior a essa atualização.

- A árvore rubro-negra adotada é LLRB, com elos vermelhos inclinados à esquerda,
  sem ponteiro de pai nem sentinela. O cabeçalho do índice identifica a variante.
- Ausentes ordenam depois dos valores presentes em todas as árvores. Tipos
  diferentes ordenam pelo valor de `TIPO_CAMPO`; cada índice deve usar um tipo.
- `ITEM` do Projeto 1 permanece uma linha temporária. O índice mantém apenas
  cópias tipadas das chaves e offsets; `ftello` pode ser capturado pelo futuro
  coordenador imediatamente antes de `reader_ler`, sem alterar CSV ou reader.
- ABB, AVL e LLRB já têm APIs locais de salvar/carregar. A serialização preserva
  pré-ordem, offsets e metadados de balanceamento; a carga confere metadados do
  dataset, tipo da chave e invariantes estruturais. O formato v2 usa larguras
  fixas e little-endian para números inteiros, offsets e bits de `double` IEEE-754.
  A identidade da base é caminho, tamanho e timestamps de modificação/metadados;
  não é hash e pode não detectar alterações que preservem esses metadados.
- O [makefile](makefile) compila a biblioteca `libindice.a` e oferece `make test`.
  A CLI e o executável `trabalho` ainda não existem.
- [indexador_ler](lib/indexador.h) captura `ftello` antes de cada registro e
  chama um callback para inserir a chave; CSV e reader permanecem inalterados.
- Há testes básicos em [tests/test_arvores.c](tests/test_arvores.c), cobrindo as
  três árvores, persistência e offsets CRLF. Ainda precisam ser compilados e
  executados; esta máquina não tem GCC/Clang nem Make disponíveis.
- A recuperação de registros e a saída por offset ainda não foram implementadas.

## Requisitos essenciais do enunciado

- Ler um registro por vez, inclusive durante a construção. Nunca carregar a base completa em memória.
- Manter um nó por chave distinta, contendo todos os offsets associados. Offset é a posição em bytes do início do registro, não o número da linha.
- Salvar e restaurar a mesma topologia, incluindo alturas/fatores da AVL e cores da rubro-negra. Reinserir pares para formar outra árvore não atende ao requisito.
- Buscar por igualdade exata. A busca simples usa um índice; a complexa usa pelo menos dois índices de colunas diferentes da mesma base e faz **união (OU)**.
- Eliminar duplicatas pelo offset, inclusive entre consultas repetidas. Linhas iguais em posições diferentes continuam sendo registros distintos.
- Recuperar resultados por posicionamento no arquivo e leitura individual. Não varrer a base para buscar chaves nem reconstruir índices no modo de busca.
- Preservar cabeçalho, colunas, ordem das colunas, delimitador e valores originais. Sem resultados, exportar só o cabeçalho.
- Implementar os três TADs próprios, gerar `trabalho` com `make`, aceitar a CLI exigida e imprimir métricas `NOME=VALOR`.

## Arquitetura recomendada e decisões iniciais

### Separar registro, chave e item do índice

O TAD chave é uma boa proposta e já está parcialmente implementado. Deve cuidar de valor tipado, validação, cópia, comparação e liberação, sem conhecer CSV, datasets, rotações ou o arquivo completo de índice.

Recomendação de evolução, ainda não implementada:

| Papel | Conteúdo e duração |
| --- | --- |
| REGISTRO temporário | Atual representação da linha, com campos, texto original e offset. Liberado após extrair a informação necessária. O esquema pertence ao leitor. |
| CHAVE | Tipo, indicador de ausência e valor próprio; textos são copiados para sobreviver ao registro temporário. |
| ITEM do índice | O nó privado (`NO`) de cada árvore contém CHAVE e coleção de offsets, além de filhos e metadados. |

Para minimizar mudanças no CSV e no reader, o ITEM herdado continua representando somente o registro corrente. O coordenador captura o offset ao redor da leitura, extrai a chave e a árvore copia apenas chave e offset; então o ITEM é liberado. Assim o nó da árvore cumpre o papel de item compacto do índice, sem guardar a linha completa. **Não acumular ITEMs nem linhas na árvore.**

Manter a coleção de offsets no ITEM ou em módulo próprio; separar métricas do TAD chave. Preservar três TADs de árvore com interfaces compatíveis. Uma fachada pequena pode selecionar a implementação na aplicação, sem fundir as árvores.

Decisões adotadas nesta etapa:

- Um tipo de chave por índice, determinado pela coluna. As árvores agora rejeitam mistura de tipos; a persistência recebe e valida o tipo declarado.
- Ausentes ficam por último em todas as árvores, alinhados a [item.c](src/item.c).
- Consultas numéricas com ponto decimal; texto exato; booleanos `true`/`false` nas consultas, embora o CSV NASA use `True`/`False`.
- Literal para consultar valores ausentes, não definido no enunciado. Uma opção é valor vazio em `KEY|coluna|`; documentar e testar antes de adotar. Ausência não é zero nem falso.
- Posse da memória: árvore possui cópias das chaves e offsets; busca empresta a coleção até mutação/destruição; o registro temporário pode ser liberado após inserção.

### Escolher conscientemente a variante rubro-negra

As [notas da parte 1](materiais_locais/notas-aulas-red-back-tree-parte1.pdf) e [parte 2](materiais_locais/notas-aula-black-tree-parte2-v2.pdf) ensinam **LLRB**, com ligações vermelhas inclinadas à esquerda. Esta é a variante adotada em [src/rubro_negra.c](src/rubro_negra.c). Inserção e validação seguem as invariantes LLRB; persistência grava cores e rejeita árvores com elo vermelho à direita ou alturas negras diferentes.

Adotar a convenção já utilizada nas árvores e na [AVL completa do professor](materiais_locais/atividade-01-avl-completa/avl.c): vazio tem altura 0, folha tem altura 1. Remoção de chaves aparece nos exercícios, mas não é exigida pelos modos index/search; não deve atrasar a entrega.

## Plano por etapas e critérios de aceite

### 1. Contratos e compilação

- [x] Registrar as decisões de representação, ausência, posse da memória e variante rubro-negra; alinhar headers antes de dividir implementação.
- [x] Corrigir include/guarda do módulo chave e definir o ITEM existente como registro temporário; o nó privado de cada árvore guarda somente chave e offsets.
- [ ] Preencher o build com C11, avisos, include de [lib/](lib/) e suporte POSIX para `ftello`, `fseeko` e relógio monotônico. Sugestão: `_POSIX_C_SOURCE=200809L` e `_FILE_OFFSET_BITS=64`.
- [ ] Criar alvos de aplicação, testes e limpeza. Compilar somente a aplicação, sem os programas principais dos materiais do professor.
- [ ] Adaptar os testes CSV/reader do Projeto 1, sem importar a arquitetura que carrega a base toda.

**Aceite:** módulos compilam sem avisos e testes básicos passam; ao introduzir o programa principal, `make` gera `trabalho`. Esses alvos ainda não existem.

### 2. Offsets e acesso direto

- [ ] Abrir entrada em modo binário, consumir o cabeçalho e capturar `ftello` imediatamente antes de cada registro. Verificar erros e guardar `off_t` no registro temporário.
- [ ] Expor acesso ao offset sem expor a estrutura interna. Não calcular posição a partir de texto decodificado ou número de linha.
- [ ] Implementar recuperação por `fseeko(..., SEEK_SET)` seguida de leitura individual, preservando os bytes originais. Tratar o estado EOF/erro persistente do leitor atual ao reposicionar.
- [ ] Usar o fluxo ler → extrair chave/offset → inserir cópia compacta → liberar registro; não acumular linhas.
- [ ] Testar LF, CRLF, CR, UTF-8, aspas, campos vazios, linhas longas e última linha sem terminador. O parser herdado também aceita campos multilinha; o enunciado assume uma linha por registro. A capacidade adicional não deve quebrar offsets.

**Aceite:** offsets permitem reler exatamente os registros, fora de ordem e após EOF; cabeçalho não é indexado; memória dos registros não cresce com a base.

### 3. Chave/ITEM e revisão das árvores

- [ ] Testar comparação/cópia de texto, inteiro, real finito, booleano e ausente, além de entradas inválidas e falhas de alocação.
- [ ] Revisar ABB com entradas crescentes, decrescentes, aleatórias e repetidas. Inserção e destruição recursivas podem esgotar a pilha em árvores degeneradas: usar algoritmos iterativos/pilha explícita onde necessário.
- [ ] Revisar AVL nos casos LL, RR, LR e RL, validando alturas e fator entre -1 e 1 em todos os nós.
- [ ] Revisar rubro-negra: raiz/NIL pretos, nenhum vermelho com filho vermelho, igual altura negra nos caminhos e ordenação de ABB. Para LLRB, verificar também inclinação à esquerda.
- [ ] Garantir que chave repetida apenas acrescenta offset, sem criar outro nó; liberar tudo sem vazamentos e preservar a estrutura em falhas.
- [ ] Contar chaves distintas separadamente de registros; uma comparação por chamada efetiva ao comparador, duas rotações em rotação dupla e recolorações apenas para mudanças efetivas, não inicialização de nó.

**Aceite:** mesmas associações chave→offsets nas três árvores; validadores passam após inserções em sequências pequenas e testes de estresse. Usar sanitizadores de memória e comportamento indefinido quando disponíveis.

### 4. Persistência fiel

- [x] Especificar formato versionado v2 com assinatura, árvore/variante, coluna, tipo da chave e identidade prática da base. Associar leitor, limite e contagens ao formato quando a aplicação definir esses dados.
- [x] Salvar nós e filhos em pré-ordem com marcadores de nulos; persistir chaves, ausências, offsets e alturas/cores, sem ponteiros nem structs brutas.
- [x] Usar larguras fixas little-endian para inteiros, comprimentos e offsets; preservar double IEEE-754 e verificar conversões para `off_t`.
- [x] Carregar diretamente a topologia, sem reinserção; a LLRB não usa pais nem sentinela.
- [x] Validar tipo, ordenação, offsets dentro do arquivo, alturas e balanceamentos, cores, inclinação à esquerda, altura negra, truncamento e bytes extras; liberar reconstruções parciais.
- [x] Usar caminho, tamanho e timestamps de modificação/metadados para compatibilidade da base; documentar que essa política não detecta toda alteração possível e não relê o CSV na busca.
- [ ] Adicionar e executar testes de ida e volta, corrupção, incompatibilidade e invariantes das três árvores.

**Aceite:** salvar → apagar → carregar preserva topologia, cores/alturas, valores e offsets; testes detectam corrupção e alteração usual da base. Carregar não incrementa métricas de inserção/rotação.

### 5. CLI, consultas e saída

- [ ] Implementar `--mode index|search`, `--reader nasa|empresas`, `--tree abb|avl|rubro_negra`, `--input`, `--key`, `--index`, `--query`, `--output` e `--limit` conforme o modo do enunciado.
- [ ] Indexação: `--limit 0` processa tudo; limite positivo processa um prefixo. Rejeitar negativos/overflow.
- [ ] Busca: aceitar `--index` repetido; exigir mesma base, colunas distintas na busca complexa e índice para toda coluna consultada.
- [ ] Interpretar `KEY|coluna|valor` pelo tipo do índice; rejeitar sintaxe/coluna inválida. Chave válida inexistente gera zero resultados, sem erro.
- [ ] Unir offsets dos resultados. Sugestão inicial: vetor, ordenação e remoção de repetidos; alternativa futura: conjunto. Nunca deduplicar pelo conteúdo da linha.
- [ ] Exportar em ordem crescente de offset, opção do projeto para determinismo, não ordem de registros explicitamente exigida no enunciado. Recuperar/escrever uma linha por vez.
- [ ] Proteger entrada e índices contra sobrescrita acidental por caminhos equivalentes. Conferir erros de leitura/escrita, `fflush` e fechamento; diagnóstico em stderr e retorno não zero em erro.

**Aceite:** busca simples/complexa nas duas bases, com existentes, inexistentes e repetidas; resultado igual entre árvores; só cabeçalho quando vazio; `STATUS=OK` apenas após conclusão bem-sucedida.

### 6. Métricas e testes integrados

Instrumentar junto com cada operação; nesta etapa, consolidar fronteiras e formato.

| Modo | Métricas |
| --- | --- |
| Indexação | `STATUS`, `TREE`, `KEY`, `INPUT_RECORDS`, `INDEXED_RECORDS`, `DISTINCT_KEYS`, `TREE_HEIGHT`, `INSERT_COMPARISONS`, `ROTATIONS`, `RECOLORS`, `BUILD_MS`, `SAVE_INDEX_MS` |
| Busca | `STATUS`, `INDEX_COUNT`, `QUERY_KEYS`, `SEARCH_COMPARISONS`, `MATCHED_OFFSETS`, `OUTPUT_RECORDS`, `LOAD_INDEX_MS`, `SEARCH_MS`, `FETCH_MS`, `WRITE_MS`, `TOTAL_MS` |

- [ ] Definir construção como leitura + inserção; salvar/carregar separadamente; `SEARCH_MS` só para buscas nas árvores, sem E/S de registros. Parsing e deduplicação ficam fora desse intervalo, mas entram no total.
- [ ] Se todas as linhas KEY válidas forem executadas, contar todas em `QUERY_KEYS`, inclusive repetidas; manter coerência com comparações reais. `MATCHED_OFFSETS` representa offsets únicos.
- [ ] Medir com relógio monotônico, em milissegundos; operações não aplicáveis imprimem zero. Separar fetch/escrita mesmo quando alternados por registro; incluir conclusão da escrita no tempo correspondente.
- [ ] Cobrir árvore vazia, só cabeçalho, limites, consultas inválidas, colisões entre índices, arquivos truncados, base alterada e erros de E/S.
- [ ] Comparar saídas com referência em fixtures pequenas. Varredura pode construir o resultado esperado no teste, nunca substituir a busca do programa.

**Aceite:** métricas coletáveis automaticamente, contagens pequenas conferidas manualmente e testes que detectam perda/duplicação de registros e reconstrução indevida do índice.

### 7. Experimentos e entrega

- [ ] Usar duas bases e três árvores, com mesmos registros, ordem de inserção, colunas e consultas em cada configuração; variar tamanhos. Dados duplicados/amostrados geram novo CSV e índices próprios.
- [ ] Busca simples com 1, 10, 100 e 1.000 chaves; complexa com 10, 100 e 1.000 chaves distribuídas em pelo menos duas colunas.
- [ ] Incluir existentes, inexistentes e repetidas; fixar semente e documentar geração. Construir índices antes dos experimentos de busca.
- [ ] Executar 10 repetições por configuração, variando ordem dos métodos e mantendo pareamento por repetição. Registrar hardware, SO, compilador, flags e condições.
- [ ] Guardar dados brutos; calcular médias e desvios padrão amostrais dos tempos, médias das contagens e diferenças percentuais com referência explícita.
- [ ] Em pelo menos duas configurações representativas, aplicar teste t pareado a 5% para ABB × AVL e AVL × rubro-negra; informar p-valor ou valor crítico, limitações e relevância prática.
- [ ] Gerar gráficos obrigatórios de construção por tamanho, busca simples e complexa por quantidade de chaves; tabelas de custo de arquivo, comparações, rotações, recolorações e altura.
- [ ] Analisar N registros, D chaves distintas, Q consultas e offsets retornados: busca O(h); ABB pode ter altura O(D), balanceadas O(log D); construção pode chegar a O(ND) na ABB e O(N log D) nas balanceadas, além de parsing/cópia. Offsets ocupam O(N); discutir também deduplicação e E/S. Para D muito pequeno, usar log(D+1) na formulação geral.
- [ ] Entregar fontes, build, consultas, exemplos reais e saídas de indexação/busca simples/complexa em ambas as bases, relatório PDF, dados brutos e scripts/planilhas de análise. Comparar saídas entre árvores.

**Aceite:** entrega reproduzível a partir dos datasets e relatório cobrindo todos os requisitos do enunciado.

## Exemplos-alvo de execução

Os arquivos de consulta abaixo ainda precisam ser criados. Após implementar o programa:

```sh
make
mkdir -p indices consultas resultados

./trabalho --mode index --reader nasa --input data/asteroids.csv --tree avl --key year --index indices/ano.avl.idx --limit 0
./trabalho --mode index --reader nasa --input data/asteroids.csv --tree rubro_negra --key on_sentry_list --index indices/sentry.rb.idx --limit 0
./trabalho --mode search --reader nasa --input data/asteroids.csv --index indices/ano.avl.idx --query consultas/anos.txt --output resultados/anos.csv
./trabalho --mode search --reader nasa --input data/asteroids.csv --index indices/ano.avl.idx --index indices/sentry.rb.idx --query consultas/nasa_multiplas.txt --output resultados/nasa_multiplas.csv

./trabalho --mode index --reader empresas --input data/mapa_empresas.csv --tree abb --key cod_ibge --index indices/municipios.abb.idx --limit 0
./trabalho --mode index --reader empresas --input data/mapa_empresas.csv --tree avl --key 'Empresas Abertas' --index indices/abertas.avl.idx --limit 0
./trabalho --mode search --reader empresas --input data/mapa_empresas.csv --index indices/municipios.abb.idx --query consultas/municipios.txt --output resultados/municipios.csv
./trabalho --mode search --reader empresas --input data/mapa_empresas.csv --index indices/municipios.abb.idx --index indices/abertas.avl.idx --query consultas/empresas_multiplas.txt --output resultados/empresas_multiplas.csv
```

Conteúdos propostos, uma consulta por linha:

| Arquivo a criar | Linhas |
| --- | --- |
| `consultas/anos.txt` | `KEY\|year\|2028` |
| `consultas/nasa_multiplas.txt` | `KEY\|year\|2028` e `KEY\|on_sentry_list\|true` |
| `consultas/municipios.txt` | `KEY\|cod_ibge\|520005` |
| `consultas/empresas_multiplas.txt` | `KEY\|cod_ibge\|520005` e `KEY\|Empresas Abertas\|74` |

O exemplo complexo de empresas mistura árvores, algo que a arquitetura deve suportar. Experimentos comparativos usarão configurações equivalentes entre métodos.

## Organização em dupla

Fechar juntos os contratos da etapa 1. Depois, um integrante pode assumir leitores/offsets, CLI e consultas; o outro, chave/ITEM, árvores e persistência. Integrar primeiro um fluxo pequeno com ABB e depois aplicar os contratos a AVL/rubro-negra. Cada integrante revisa os módulos do outro; experimentos dependem da integração validada.

Combinar alterações nos headers compartilhados. Cada entrega deve informar arquivos afetados, testes executados e limitações. Marcar etapas somente após cumprir o aceite e manter [AGENTS.md](AGENTS.md) coerente.

## Datasets e materiais

- [Asteroides](data/asteroids.csv): esquema de 20 colunas; campos numéricos vazios são ausentes; preservar os valores existentes na base.
- [Mapa de Empresas](data/mapa_empresas.csv): 17 colunas com blocos municipal e estadual independentes lado a lado. Não relacionar município/estado pela linha. Campos estaduais usam prefixo interno `estado.`, separadora `_separador`, códigos IBGE são texto e `-` no tempo médio significa ausência.
- [Materiais locais](materiais_locais/) são referência didática, não fontes da aplicação. Consultar [ABB](materiais_locais/05-abb-altura-balanceamento/), [AVL completa](materiais_locais/atividade-01-avl-completa/) e notas rubro-negras. Não modificar os exemplos para corrigir a aplicação.
