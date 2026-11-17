# projeto-compilador

Projeto de Compiladores e Linguagens Formais Autômatos — **KomboScript**, uma
linguagem de combos para jogo de luta (ver `docs/especificacao.md`).

## Como rodar os testes

Um único comando, na raiz do repositório (precisa de CMake ≥ 3.14 e de um
compilador com C++20):

```
cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure
```

Compila com `-Wall -Wextra -Wpedantic -Werror` (`/W4 /WX` no MSVC), roda um
teste por parte e termina com código diferente de zero se qualquer verificação
falhar. O `ctest` deve terminar com `100% tests passed, 0 tests failed out of 4`.

Para ver as contagens que cada teste imprime (incluindo a tabela de
crescimento de estados), rode o programa do teste direto, por exemplo
`./build/test_minimizacao`.

## Estrutura

```
CMakeLists.txt
include/            cabeçalhos
  automato.h          Estado, Simbolo, Transicao e as marcas reservadas
  expressao.h         leitura da expressão -> árvore reduzida ao núcleo
  afn.h               AFN: estrutura, fecho vazio, movimento, simulação
  afd.h               AFD (tabela de transição densa)
  thompson.h          árvore -> AFN
  determinizacao.h    AFN -> AFD (subconjuntos)
  minimizacao.h       AFD -> AFD mínimo (refinamento de partições)
src/                implementações (um .cpp por cabeçalho, exceto automato.h)
tests/              um teste por parte, mais os apoios
  test_thompson.cpp, test_afn.cpp, test_determinizacao.cpp, test_minimizacao.cpp
  casos.txt           a bateria conferida à mão (72 casos)
  bateria.h/.cpp      leitor e executor da bateria
  comparacao.h/.cpp   as três máquinas sobre todas as cadeias até um comprimento
  verificar.h, ajuda.h  verificações e utilitários dos testes
demos/              programas de demonstração dos marcos 02 e 03
docs/, especificacao/, exemplos/   documentação da linguagem
main.cpp            do padrão ao AFD mínimo (alvo `kombo`)
```

Exemplo do programa principal:

```
echo "a b a b a c" | ./build/kombo "[a | b]* -> c" a b c
```

## Onde está cada parte

| Parte | Arquivos |
| --- | --- |
| Árvore da expressão (ponto de partida) | `include/expressao.h`, `src/expressao.cpp` |
| Construção de Thompson (árvore → AFN) | `include/thompson.h`, `src/thompson.cpp` |
| Fecho vazio, movimento e simulação do AFN | `include/afn.h`, `src/afn.cpp` (`fechoVazio`, `mover`, `simular`, `aceita`) |
| AFD (tabela de transição densa) | `include/afd.h`, `src/afd.cpp` |
| Determinização (subconjuntos) | `include/determinizacao.h`, `src/determinizacao.cpp` |
| Minimização (refinamento de partições) | `include/minimizacao.h`, `src/minimizacao.cpp` |
| Bateria conferida à mão (72 casos) | `tests/casos.txt`, lida por `tests/bateria.cpp` |
| Comparação das três máquinas | `tests/comparacao.h`, `tests/comparacao.cpp` |
| Teste de Thompson | `tests/test_thompson.cpp` (teste `thompson`) |
| Teste do AFN e da bateria | `tests/test_afn.cpp` (teste `afn`) |
| Teste da determinização | `tests/test_determinizacao.cpp` (teste `determinizacao`) |
| Teste da minimização, das três máquinas e do crescimento | `tests/test_minimizacao.cpp` (teste `minimizacao`) |

A construção de Thompson recebe a **árvore** que `analisarExpressao` produz,
não o texto. Cobre os três operadores do núcleo (`->`, `|`, `*`), o símbolo e a
cadeia vazia; `+`, `?` e `[ ]` já chegam reduzidos ao núcleo (ver
`docs/nucleo_minimo.md`), então nenhum operador reconhecido fica de fora.
`a.c` do guia usa um coringa, que a linguagem não tem, e por isso não é testado.

O `CMakeLists.txt` da raiz substitui o esboço antigo `marcos/01/CMakeList.txt`
(nome errado e fonte `src/01_marco.cpp` inexistente), cujos avisos de
compilação foram mantidos. A pasta `marcos/` deixou de existir; sobrou nela
só `marcos/03/ex_afd.cpp`, uma cópia antiga de outro projeto (namespace
`peneira`, inclui `03_afd.h`, que não existe) que nunca compilou aqui e fica
fora do build.

### Notação

Os padrões do guia aparecem na notação do KomboScript: `(a|b)*c` é
`[a | b]* -> c`.

## Representação do conjunto vazio

O `AfdCombo` do projeto já tem um estado de erro absorvente (o último índice) e
função de transição total. A determinização usa esse estado como o conjunto
vazio: quando nenhum estado ativo do AFN tem transição para o símbolo, o destino
é o erro. Por isso o AFD sai **completo**, e o passo "completar o AFD" da
minimização já vale por construção. Em `ResultadoDaDeterminizacao`,
`conjuntos[i]` guarda os estados do AFN que o estado `i` representa, e
`quantidadeDeEstados()` conta o vazio só quando ele é de fato alcançado — a
mesma contagem do guia.

## Resultados conferidos

`(a|b)*c` sobre {a, b, c}:

| Verificação | Resultado |
| --- | --- |
| AFN de Thompson (estados / transições / vazias) | 10 / 12 / 9 |
| `ababac` / `abab` | aceita / rejeita |
| AFD pela determinização (vazio contado) | 5 estados |
| Estados antes → depois da minimização | 5 → 3 (mínimo completo, contado o erro) |
| Cadeias até o comprimento 4 | 121 |
| Aceitas por AFN, AFD e mínimo | 15, 15 e 15 |

Outros padrões (AFN → AFD → mínimo): `(a|b)*abb` 14 → 5 → 4, `a*` 4 → 3 → 2,
`(a|b)*` 8 → 3 → 1, `a+b` 8 → 5 → 4. O AFD do Hadouken montado à mão no
marco 03 já é mínimo (5 → 5), e um AFD montado à mão com estado inacessível e
estados equivalentes cai de 4 alcançáveis para 2.

## Crescimento de estados na determinização

Padrão: **"o n-ésimo símbolo a partir do fim é `a`"**, escrito como
`[a | b]* -> a -> [a | b] -> ... -> [a | b]` (com n − 1 repetições de
`[a | b]` no fim). Alfabeto {a, b}. Contagens impressas pelo programa:

| n | AFN | AFD (subconjuntos) | AFD mínimo | 2^n |
| ---: | ---: | ---: | ---: | ---: |
| 1 | 10 | 3 | 2 | 2 |
| 2 | 16 | 5 | 4 | 4 |
| 3 | 22 | 9 | 8 | 8 |
| 4 | 28 | 17 | 16 | 16 |
| 5 | 34 | 33 | 32 | 32 |
| 6 | 40 | 65 | 64 | 64 |
| 7 | 46 | 129 | 128 | 128 |
| 8 | 52 | 257 | 256 | 256 |
| 9 | 58 | 513 | 512 | 512 |
| 10 | 64 | 1025 | 1024 | 1024 |
| 11 | 70 | 2049 | 2048 | 2048 |
| 12 | 76 | 4097 | 4096 | 4096 |

**Padrão observado:**

- O AFN cresce **linearmente**: 6n + 4 estados.
- O AFD cresce **exponencialmente**: 2^n + 1 estados. Cada subconjunto
  alcançável corresponde à janela das n últimas letras lidas (2^n janelas). O
  "+1" é o subconjunto inicial da construção de Thompson, que nunca se repete.
- A minimização elimina só esse "+1": o subconjunto inicial é equivalente ao
  estado "só `b`". Sobram **2^n**, e não dá para reduzir mais: cada estado
  lembra as n últimas letras, então todos são distinguíveis entre si.
- Conclusão: o custo exponencial não é artefato da construção, é intrínseco à
  linguagem — todo AFD que a reconhece precisa de pelo menos 2^n estados.
  Nenhum conjunto vazio é alcançado aqui, porque `[a | b]*` sempre tem transição.
  O programa também confere que as três máquinas concordam em todas as cadeias
  até o comprimento 10, para n ≤ 8.
