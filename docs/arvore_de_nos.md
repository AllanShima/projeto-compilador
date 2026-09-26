## 1. Tabela de Peças

| Nome da Peça     | Trecho Real da Especificação | Padrão com Açúcar           | Padrão no Núcleo                                     | Forma Linear da Árvore                                      | Nós |
| :--------------- | :--------------------------- | :-------------------------- | :--------------------------------------------------- | :---------------------------------------------------------- | :-- |
| **Mashing**      | `ChuteFraco -> ChuteFraco+`  | `ChuteFraco -> ChuteFraco+` | $$ChuteFraco \cdot (ChuteFraco \cdot ChuteFraco^*)$$ | `concat(ChuteFraco, concat(ChuteFraco, fecho(ChuteFraco)))` | 6   |
| **Sinal**        | `->`                         | `->`                        | $$- \cdot >$$                                        | `concat('-', '>')`                                          | 3   |
| **Opcional**     | `?`                          | `?`                         | $$\text{'?'}$$                                       | `alt(x, eps)`                                               | 3   |
| **Palavra Fixa** | `arena`                      | `arena`                     | $$a \cdot r \cdot e \cdot n \cdot a$$                | `concat('a', concat('r', concat('e', concat('n', 'a'))))`   | 9   |

---

## 2. Cobertura Mínima

A KomboScript utiliza operadores de repetição e opcionalidade na especificação de macros e combos. O operador de fecho positivo (`+`) é aplicado na construção de sequências repetidas de comandos, como em `Mashing = ChuteFraco -> ChuteFraco+;`.

Além disso, a opcionalidade (`?`) é usada na definição de modificadores de golpes que podem ou não ser incluídos pelo jogador em uma sequência, como em comandos que um direcional pode ser omitido pelo jogador sem invalidar o reconhecimento do combo.

---

## 3. Um Par que Converge

Duas escritas diferentes que denotam o mesmo conjunto de sequências devem resultar na mesma árvore e na mesma contagem de nós.

- **Expressão A:** `ChuteFraco -> ChuteFraco+`
- **Expressão B:** `ChuteFraco -> ChuteFraco -> ChuteFraco*`

Ambas as expressões exigem ao menos duas repetições de `ChuteFraco`. Quando reduzidas ao núcleo, elas produzem a mesma forma linear, `concat(ChuteFraco, concat(ChuteFraco, fecho(ChuteFraco)))`, correspondendo a 6 nós na árvore reduzida.

---

## 4. Um Par que Não Concorre

A comparação de árvores sintáticas reduzidas tem limitações quanto à distribuição dos operadores, o que significa que expressões semanticamente equivalentes podem gerar estruturas de nós totalmente diferentes.

- **Expressão A:** `(RolarFrente | RolarTras)*`
- **Expressão B:** `(RolarFrente* -> RolarTras*)*`

Ambas as expressões aceitam qualquer sequência formada por repetições e combinações de `RolarFrente` e `RolarTras`. Mas a árvore da primeira expressão mostra um fecho aplicado diretamente sobre uma alternância simples, enquanto a segunda gera um encadeamento aninhado de fechos em subsequências independentes.

Isso demonstra que a análise baseada apenas na árvore reduzida não serve como provador de equivalência de linguagens. O comparador estrutural mostrará divergência de formas, já que a igualdade detectada se restringe à sintaxe imediata da redução, exigindo a construção de autômatos mínimos para verificar a equivalência semântica.

---

## 5. Dois Requisitos, Um de Cada Lado

### Requisito Resolvido pela Classe

A validação de intervalos numéricos específicos que parecem exigir lógica aritmética complexa pode ser resolvida diretamente por padrões regulares estruturados. Um exemplo no nosso projeto é a **validação da janela de cancelamento de quadros de animação (frames de tolerância de 0 a 60)**.

Embora pareça exigir uma operação relacional ($frame \le 60$), o conjunto de valores é finito e resolvido inteiramente na fase léxica pela expressão regular:
`([0-5]?[0-9]|60)`

_(Onde a máquina de estados só precisa registrar em qual posição do caractere numérico está no momento, sem necessidade de memória extra)._

### Requisito Fora da Classe

A validação e reconhecimento de combos baseados no **pareamento e balanceamento estrito de cancelamentos encadeados de comandos (ex: número ilimitado de entradas de ataque seguidas do mesmo número de confirmações de bloqueio)** excede a capacidade de um autômato finito.

Para verificar essa condição, a máquina **precisaria armazenar um contador dinâmico ou utilizar uma pilha de memória** para guardar o número de entradas já lidas e comparar com as saídas. Como a profundidade da sequência de combo escrita pelo usuário não possui um limite máximo fixo, essa verificação não pode ser feita por expressões regulares, exigindo um analisador sintático com pilha (Linguagem Livre de Contexto).

---

## 6. Quatro Recusas com Posição

1. **Falta de fechamento de grupo:**
   - _Mensagem:_ `posição 18: colchete aberto na posição 12 nunca é fechado`
2. **Repetição sem operando:**
   - _Mensagem:_ `posição 6: "*" repete o que vem antes, e não há nada antes dele`
3. **Fecha-colchete sem abertura correspondente:**
   - _Mensagem:_ `posição 4: fecha-colchete sem abertura correspondente`
4. **Símbolo sobrando após o fim da expressão:**
   - _Mensagem:_ `posição 45: token inesperado encontrado após o término da declaração válida`
