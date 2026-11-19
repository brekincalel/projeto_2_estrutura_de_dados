# Orientações para agentes — SCC0202, Projeto 2

## Contexto e fontes

Este é o segundo trabalho de Algoritmos e Estruturas de Dados I: índices persistidos em ABB, AVL e rubro-negra, com chaves de colunas e offsets de CSV. Comunicar-se em português, explicando escolhas no contexto acadêmico.

Leia o [plano e critérios de aceite](README.md). A fonte dos requisitos é o [enunciado](materiais_locais/projeto2.pdf); recomendações do plano não são automaticamente exigências do professor. Se o PDF estiver ausente em outro checkout, usar o resumo do README e sinalizar a ausência antes de interpretar detalhes não documentados.

Projeto 1: https://github.com/LuizCunha4/projeto_1_estrutura_de_dados — revisão consultada `98f4cd56088971ccda40d6262fddc1be7861ece5`. Foram reutilizados CSV, ITEM e leitores. Não importar o fluxo que carrega a base inteira nem FILTER/SIM/ORDER/GROUP/CROSS: aqui são consultas `KEY|coluna|valor` e acesso por offset. A cópia temporária usada no planejamento não é dependência do projeto.

## Estado inicial registrado em 19/11/2026

- Existem inserção/busca nas três árvores e módulo comum [chave.h](lib/chave.h)/[chave.c](src/chave.c), com listas de offsets e métricas. Não presumir que é preciso começar do zero nem que estão certificadas.
- [src/chave.c](src/chave.c) inclui `arvore_comum.h`, inexistente; [lib/chave.h](lib/chave.h) mantém guarda antiga. A verificação de sintaxe falhou nesse include.
- [makefile](makefile) vazio; ainda não há programa principal, persistência, parser de consultas ou suíte de testes. Não afirmar que o programa ou testes completos passaram.
- [reader.c](src/reader.c) já entrega um registro por vez, mas não fornece offsets/acesso aleatório. EOF/erro persistem no seu estado.
- [ITEM](lib/item.h) representa linha completa e conserva contratos do Projeto 1. O plano propõe migrá-lo para REGISTRO temporário e reservar ITEM para chave + offsets; essa migração ainda não ocorreu.
- ITEM coloca ausentes por último; chave coloca por primeiro. Unificar a regra antes da integração.
- Rubro-negra atual é clássica, com pai e NIL; notas do professor são LLRB. Escolher/documentar variante antes de reescrever ou definir persistência. Não misturar algoritmos.
- [.gitignore](.gitignore) vazio; materiais locais já rastreados, inclusive binários. Não afirmar que estão ignorados nem removê-los do Git sem escopo autorizado.
- Havia alterações prévias nos três headers das árvores e arquivos não rastreados. Preservar trabalho do colega e verificar estado atual. Atualizar este retrato conforme houver implementação.

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

- CSV: sintaxe e bytes originais. Leitor: esquema/dataset. CHAVE: valor tipado. ITEM compacto: chave e offsets. Árvores: estrutura/balanceamento. Persistência: formato. Aplicação: CLI e coordenação.
- Não transformar chave em módulo genérico de CSV, CLI e rotações. Preferir tipos opacos e contratos explícitos nos headers.
- Leitor possui esquema, mas não fecha FILE recebido; registros dependentes devem morrer antes do leitor. Árvore copia valores necessários e não depende do registro após inserção.
- Retornos de busca são emprestados: não liberar/modificar vetor interno; documentar validade perante inserções e destruição.
- Decisões pendentes: nomes REGISTRO/ITEM, variante rubro-negra, literal e ordem de ausentes, política de validação da base. Seguir recomendações do README quando apropriado e registrar desvios; continuar tarefas independentes dessas decisões.
- Manter números/booleanos tipados e igualdade exata de reais; rejeitar não finitos/conversões inválidas. Códigos IBGE são texto.
- NASA usa `True`/`False` no CSV; consultas usam `true`/`false`. Empresas tem blocos independentes, prefixo interno `estado.`, separadora `_separador` e ausência `-` no tempo médio.

## Implementação e validação

Seguir dependências do README: contratos/build → offsets → árvores/itens → persistência → CLI/buscas → validação → experimentos. Remoção de chaves e comandos do Projeto 1 não são pré-requisitos.

Antes de editar, ler headers/implementações envolvidos e executar `git status --short`. Não reverter alterações alheias. Mudanças de assinatura exigem atualizar declarações, definições, consumidores e testes juntos; combinar alterações nos headers compartilhados com o colega.

Diagnóstico inicial, ainda falhando no include descrito:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -Ilib -fsyntax-only src/*.c
```

O build futuro deve produzir `trabalho` e habilitar suporte POSIX/offsets grandes conforme README. Depois de implementado, usar `make` e alvos de teste realmente existentes; não apresentar alvos planejados como disponíveis. Não compilar os programas principais de [materiais_locais/](materiais_locais/) na aplicação.

Testar especialmente offsets CRLF/UTF-8, duplicatas, LL/RR/LR/RL, invariantes da variante rubro-negra escolhida, ABB degenerada/estouro de pilha, restauração de topologia idêntica, índices truncados/incompatíveis, consultas sobrepostas/repetidas, saída vazia e resultados iguais entre árvores. Usar sanitizadores quando disponíveis. Testes CSV/reader do Projeto 1 são candidatos a adaptação, não prova das novas APIs.

Não alterar materiais didáticos para corrigir a aplicação nem executar seus binários precompilados como validação do código do grupo. Combinar política de dados e artefatos; não versionar índices/build/resultados volumosos por acidente.

Ao concluir etapas, informar mudanças, verificações e limitações; atualizar README e este arquivo. Não marcar planejamento como implementação concluída. Experimentos exigem duas bases, três árvores, 10 repetições, consultas fixas e testes t pareados conforme o plano. Nunca fabricar medições.
