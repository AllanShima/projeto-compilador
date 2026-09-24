# Que classe de padrões o sistema aceita

**Decisão:** movimentos e ataques podem se juntar em combos através dos três
operadores de expressão regular: concatenação (`->`), alternância (`|`) e fecho
(`*`). Um `combo` ou `macro` pode compor outros combos e macros já declarados
dentro da sua própria expressão, sem limite de profundidade.

**Núcleo mínimo:** os três operadores acima — concatenação, alternância e fecho.

**Conveniência de escrita:** o fecho positivo (`+`) é aceito na sintaxe, mas
**reduzido ao núcleo** antes de qualquer processamento — `X+` vira `X -> X*`.
A redução acontece uma única vez, logo depois da leitura, e tudo o que vem
depois trabalha só com os três operadores do núcleo.

# O alfabeto inclui um estado neutro explícito

**Decisão:** o alfabeto de entrada tem um símbolo `Neutro`, que representa
um quadro em que o jogador não está pressionando nenhum botão de movimento ou
ataque. `Neutro` é um símbolo como qualquer outro do alfabeto e pode ser
referenciado dentro de expressões de `macro` e `combo`, como em `macro Parado = Neutro;`.

**Motivo:** alguns combos precisam distinguir um ataque solto de um ataque
encadeado. Sem um símbolo para o estado neutro, não tem como diferenciar
"nenhum input" de "input que não me interessa".

**Descartado:** tratar ausência de input como ausência de símbolo (nenhum
token emitido pelo controle). Isso impediria qualquer combo de exigir ou
permitir explicitamente um estado de repouso, porque não haveria símbolo
algum para casar com ele.

# Que forma tem a descrição escrita pelo usuário

**Decisão:** um programa é uma sequência de `macro` ou `combo` seguida
de um bloco `arena` (main). Cada declaração associa um nome a uma expressão
regular sobre movimentos e ataques, podendo referenciar macros ou combos já
declarados. Cada ação dentro de `arena` reage a um combo nomeado e produz
saída por `emit`.

# O que o sistema produz

**Decisão:** o objeto gerado tem duas partes — um vetor de autômatos finitos
determinísticos, um por `macro` ou `combo`, na forma de tabelas de transição; e,
para cada `arena`, um bytecode de máquina de pilha que executa o `emit`. Uma
máquina virtual própria varre a entrada, aplica os autômatos com desempate por
casamento mais longo e executa o bytecode.

# Descartado nesta etapa

- **Timing:** exigiria contar quadros sem teto com memória finita.
- **Assistentes:** trocaria um fluxo de entrada por dois fluxos concorrentes,
  fora do modelo de um autômato por combo.
- **Troca de personagem:** tornaria o conjunto de combos válidos dependente de
  estado externo, quebrando a ideia de autômatos compilados uma vez.
