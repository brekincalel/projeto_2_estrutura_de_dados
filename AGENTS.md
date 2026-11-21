# Orientações para agentes — SCC0202, Projeto 2

## Contexto e fontes

Este é o segundo trabalho de Algoritmos e Estruturas de Dados I: índices persistidos em ABB, AVL e rubro-negra, com chaves de colunas e offsets de CSV. Comunicar-se em português, explicando escolhas no contexto acadêmico.

Leia o [plano e critérios de aceite](README.md). A fonte dos requisitos é o [enunciado](materiais_locais/projeto2.pdf); recomendações do plano não são automaticamente exigências do professor. Se o PDF estiver ausente em outro checkout, usar o resumo do README e sinalizar a ausência antes de interpretar detalhes não documentados.

Projeto 1: https://github.com/LuizCunha4/projeto_1_estrutura_de_dados — revisão consultada `98f4cd56088971ccda40d6262fddc1be7861ece5`. Foram reutilizados CSV, ITEM e leitores. Não importar o fluxo que carrega a base inteira nem FILTER/SIM/ORDER/GROUP/CROSS: aqui são consultas `KEY|coluna|valor` e acesso por offset. A cópia temporária usada no planejamento não é dependência do projeto.

## Histórico anterior à implementação atual

Em 19/11/2026, antes do fast-forward `de2a5fc` e da implementação posterior, o
repositório tinha include quebrado, makefile vazio, rubro-negra clássica e não
possuía persistência ou testes. Esse retrato é apenas histórico e não deve ser
usado como diagnóstico do checkout atual.

Os materiais locais já rastreados, inclusive binários didáticos, continuam no
repositório. Não removê-los do Git sem escopo autorizado e não executar seus
binários precompilados como validação da aplicação.

## Invariantes obrigatórias

1. Na indexação, manter somente o registro corrente além do índice. Árvores contêm chaves e offsets, nunca linhas completas de toda a base.
2. Offset é `off_t`, em bytes, capturado por `ftello` antes do registro e após o cabeçalho. Abrir em modo binário; verificar erros. Recuperar por `fseeko` e leitura individual.
3. Um nó por chave distinta, todos os offsets preservados; strings da chave sobrevivem à liberação do registro temporário.
4. Busca por igualdade exata; complexa é união, não interseção. Deduplicar pelo offset, não pelo conteúdo da linha.
5. Busca carrega índices existentes, sem reconstrução nem varredura do CSV para localizar chaves. Pode manter offsets dos resultados em memória, não todas as linhas.
6. Persistência preserva topologia, chaves, offsets e metadados de balanceamento. Não salvar ponteiros utilizáveis nem restaurar por reinserção. Validar formato versionado e compatibilidade com a base.
7. Saída conserva cabeçalho e representação original das colunas; não adicionar offset ao CSV. Sem resultados, só cabeçalho.
8. Manter três TADs próprios e interfaces compatíveis, sem biblioteca substituindo árvores.
9. Altura: vazio 0, folha 1. Rotação dupla conta duas; recoloração conta mudança efetiva. Operações não aplicáveis imprimem zero.
10. Separar tempos de construção, gravação, carga, busca, recuperação e escrita. `SEARCH_MS` não inclui E/S dos registros. Contagens refletem operações reais.

## Responsabilidades e memória

- CSV: sintaxe e bytes originais. Leitor e `ITEM` herdado: esquema/dataset e registro temporário. CHAVE: valor tipado e lista de offsets. Nós privados das árvores: chave, offsets, estrutura e balanceamento. Persistência: formato. Aplicação: CLI e coordenação.
- Não transformar chave em módulo genérico de CSV, CLI e rotações. Preferir tipos opacos e contratos explícitos nos headers.
- Leitor possui esquema, mas não fecha FILE recebido; registros dependentes devem morrer antes do leitor. Árvore copia valores necessários e não depende do registro após inserção.
- Retornos de busca são emprestados: não liberar/modificar vetor interno; documentar validade perante inserções e destruição.
- Decisões pendentes: literal para consulta de ausentes e política mais forte de validação da base. Variante LLRB, ausência por último e manutenção do ITEM do Projeto 1 como registro temporário foram adotadas no README; não reabrir sem novo pedido do grupo.
- Manter números/booleanos tipados e igualdade exata de reais; rejeitar não finitos/conversões inválidas. Códigos IBGE são texto.
- NASA usa `True`/`False` no CSV; consultas usam `true`/`false`. Empresas tem blocos independentes, prefixo interno `estado.`, separadora `_separador` e ausência `-` no tempo médio.

## Implementação e validação

Seguir dependências do README: contratos/build → offsets → árvores/itens → persistência → CLI/buscas → validação → experimentos. Remoção de chaves e comandos do Projeto 1 não são pré-requisitos.

Antes de editar, ler headers/implementações envolvidos e executar `git status --short`. Não reverter alterações alheias. Mudanças de assinatura exigem atualizar declarações, definições, consumidores e testes juntos; combinar alterações nos headers compartilhados com o colega.

Verificação básica disponível:

```sh
make test
```

Em 20/11/2026, esse alvo compilou todos os módulos sem avisos e terminou com
`test_arvores: ok`. O [makefile](makefile) já habilita C11, POSIX e offsets
grandes e produz `libindice.a`; ainda falta o programa principal e o executável
obrigatório `trabalho`. Não apresentar os testes básicos como certificação da
aplicação completa nem compilar os programas principais de
[materiais_locais/](materiais_locais/) na aplicação.

Testar especialmente offsets CRLF/UTF-8, duplicatas, LL/RR/LR/RL, invariantes da variante rubro-negra escolhida, ABB degenerada/estouro de pilha, restauração de topologia idêntica, índices truncados/incompatíveis, consultas sobrepostas/repetidas, saída vazia e resultados iguais entre árvores. Usar sanitizadores quando disponíveis. Testes CSV/reader do Projeto 1 são candidatos a adaptação, não prova das novas APIs.

Não alterar materiais didáticos para corrigir a aplicação nem executar seus binários precompilados como validação do código do grupo. Combinar política de dados e artefatos; não versionar índices/build/resultados volumosos por acidente.

Ao concluir etapas, informar mudanças, verificações e limitações; atualizar README e este arquivo. Não marcar planejamento como implementação concluída. Experimentos exigem duas bases, três árvores, 10 repetições, consultas fixas e testes t pareados conforme o plano. Nunca fabricar medições.

## Estado atual em 20/11/2026

- O fast-forward foi integrado e trouxe `data/asteroids.csv`, `data/mapa_empresas.csv`, o plano atualizado e este arquivo.
- A chave usa `lib/chave.h`; comparador e árvores usam ausentes por último. As três árvores rejeitam chaves de tipo diferente da primeira chave inserida.
- A árvore rubro-negra agora é LLRB recursiva, sem pai ou sentinela NIL. A persistência declara variante, coluna e tipo de chave.
- Persistência v2 guarda pré-ordem, chaves, offsets, altura ABB/AVL e cores LLRB; inteiros e offsets usam codificação little-endian de largura fixa, reais exigem double IEEE-754. A carga confere ordenação, alturas/balanceamento, inclinação LLRB, altura negra e limites dos offsets.
- A base é identificada por caminho, tamanho e timestamps do sistema de arquivos, não por hash. É possível não detectar alteração que preserve esses metadados.
- CSV, reader e `ITEM` do Projeto 1 não foram alterados. `src/indexador.c` captura offsets ao redor de `reader_ler` via callback. O build dos módulos e testes básicos foram adicionados: `make` cria `libindice.a`; `make test` compila e executa `tests/test_arvores.c`.
- `make test` foi executado com sucesso neste ambiente em 20/11/2026. A suíte cobre inserção/busca básica nas três árvores, chaves repetidas, rejeição de tipos mistos, ida e volta da persistência, algumas incompatibilidades, truncamento e captura de offsets CRLF.
- Ainda faltam CLI, consultas, recuperação dos registros por offset, saída CSV, métricas da aplicação, testes abrangentes e experimentos.
