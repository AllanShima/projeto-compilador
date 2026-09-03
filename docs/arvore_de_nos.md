## 1. Tabela de Peças

| Nome da Peça     | Trecho Real da Especificação | Padrão com Açúcar | Padrão no Núcleo                                                          | Forma Linear da Árvore                                    | Nós  |
| :--------------- | :--------------------------- | :---------------- | :------------------------------------------------------------------------ | :-------------------------------------------------------- | :--- |
| **Comando**      | `Baixo -> Frente -> Jab`     | `[A-Za-z]+`       | $$(A\vert{}B\vert{}...\vert{}Z\vert{}a\vert{}b\vert{}...\vert{}z) \cdot (A\vert{}B\vert{}...\vert{}Z\vert{}a\vert{}b\vert{}...\vert{}z)^*$$ | `concat(alt('A',alt('B',...)), fecho(alt('A',...)))`      | 126  |
| **Sinal**        | `->`                         | `->`              | $$- \cdot >$$                                                        | `concat('-', '>')`                                        | 3    |
| **Número**       | `80`                         | `[0-9]+`          | $$D \cdot D^*$$                                                  | `concat(alt('0',alt('1',...)), fecho(alt('0',...)))`      | 40   |
| **Opcional**     | `?`                          | `?`               | $$\text{'?'}$$                                                             | `alt(x, eps)`                                             | 3    |
| **Palavra Fixa** | `arena`                      | `arena`           | $$a \cdot r \cdot e \cdot n \cdot a$$                                           | `concat('a', concat('r', concat('e', concat('n', 'a'))))` | 9    |

---

## 2. Cobertura Mínima

A KomboScript utiliza operadores de repetição e opcionalidade na especificação de macros e combos. O operador de fecho positivo (`+`) é aplicado na construção de sequências repetidas de comandos, como em `Mashing = ChuteFraco+;`. 

Além disso, a opcionalidade (`?`) é para a definição de modificadores de golpes que podem ou não ser incluídos pelo jogador em uma sequência, como em comandos que um direcional pode ser omitido pelo jogador sem invalidar o reconhecimento do combo.

---

## 3. Um Par que Converge

Duas escritas diferentes que denotam o mesmo conjunto de caracteres devem resultar na mesma árvore e na mesma contagem de nós.

* **Expressão A:** `[0-9]+`
* **Expressão B:** `[0-9][0-9]*`

Ambas as expressões descrevem números que podem aparecer várias vezes. Quando reduzidas ao núcleo utilizando concatenação, alternância e fecho, elas produzem a forma linear, correspondendo a 40 nós na árvore reduzida.

---

## 4. Um Par que Não Concorre

A comparação de árvores sintáticas reduzidas têm limitações de como os operadores são distribuídos, o que significa que expressões semanticamente equivalentes podem gerar estruturas de nós totalmente diferentes.

* **Expressão A:** `(a|b)*`
* **Expressão B:** `(a*b*)*`

Ambas as expressões apresentam exatamente o mesmo conjunto de cadeias formadas por combinações dos símbolos `a` e `b`. Mas a árvore gerada pela primeira expressão mostra em um fecho aplicado diretamente em uma alternância simples, enquanto a segunda gera um encadeamento aninhado de fechos em subcadeias independentes. 

Isso demonstra que a análise baseada apenas na árvore reduzida não serve como um provador de equivalência de linguagens. O comparador estrutural mostrará uma divergência de formas, uma vez que a igualdade detectada se restringe estritamente à sintaxe imediata da redução, exigindo a construção de autômatos mínimos para verificar a equivalência semântica.

---

## 5. Dois Requisitos, Um de Cada Lado

### Requisito Resolvido pela Classe
A validação de intervalos numéricos que parecem exigir lógica aritmética complexa pode, muitas vezes, ser resolvida diretamente por meio de padrões regulares estruturados. Um exemplo prático na especificação de motores de jogo é a validação de quadros de animação ou frames de tolerância de input, que podem ser confinados a um escopo estrito através de expansão de classes e concatenações direcionadas, sem a necessidade de recorrer a estruturas de controle de fluxo externas ou contadores dinâmicos durante a fase de análise léxica.

### Requisito Fora da Classe
O reconhecimento de combos baseados na contagem estrita e ilimitada de um número variável de socos e chutes precedentes excede a capacidade de um autômato finito. Para que o sistema verifique se o número de comandos acumulados em uma sequência arbitrária atende a uma condição de igualdade dinâmica, a máquina precisaria dispor de uma memória com capacidade de crescimento ilimitado, característica incompatível com a estrutura de estados finitos descrita pelas expressões regulares e árvores de análise léxica tradicionais.

---

## 6. Quatro Recusas com Posição

1. **Falta de fechamento de grupo:**
   * *Mensagem:* `posição 18: colchete aberto na posição 12 nunca é fechado`
2. **Repetição sem operando:**
   * *Mensagem:* `posição 6: "*" repete o que vem antes, e não há nada antes dele`
3. **Classe sem colchete final:**
   * *Mensagem:* `posição 22: esperava "]" para fechar a classe de caracteres`
4. **Símbolo sobrando após o fim da expressão:**
   * *Mensagem:* `posição 45: token inesperado encontrado após o término da declaração válida`
