# A representação da função de transição — decisão e a conta

## O que foi escolhido

**Matriz densa**, indexada por estado e por coluna de símbolo. Cada
combo/macro do KomboScript compila para uma tabela desse tipo; a coluna sai
de um mapeamento identificador → coluna, calculado uma vez por autômato.
Consultar uma transição custa uma multiplicação, uma soma e um acesso a
vetor — o mesmo custo em toda posição, o que permite prometer tempo
proporcional ao número de inputs do jogador sem medir nada.

A função é total: existe um estado de erro absorvente, e toda posição da
tabela nasce apontando para ele.

## A conta

Seja `n` o número de estados úteis e `m` o tamanho do alfabeto de símbolos
primitivos do jogo (movimentos e ataques — não caracteres de texto).

    bytes = (n + 1) × m × sizeof(Estado)

Para o combo `Hadouken` (`RolarFrente -> Jab`, expandido a
`Baixo -> Frente -> Jab`), com Σ = 9 símbolos primitivos e 4 estados úteis
(`inicio`, `viu_baixo`, `viu_baixo_frente`, `completo`):

    bytes = (4 + 1) × 9 × 8 = 360 bytes

Das 45 posições da tabela, 31 (69%) apontam para o erro — proporção
esperada, já que um combo específico usa só uma fração pequena do
vocabulário total do jogo a cada estado.

## A alternativa descartada

**Mapa esparso** (tabela de dispersão de pares (estado, símbolo) → destino,
com o erro implícito na ausência de chave). Ganharia em memória — só 14
entradas contra 45 —, mas foi descartado por dois motivos que não são
memória:

1. **Custo por símbolo consumido**: a VM consulta a transição uma vez por
   input do jogador, a operação mais frequente do sistema. Trocar acesso a
   vetor por hash multiplica esse custo e tira a previsibilidade linear.
2. **Formato de serialização**: a tabela é o que o objeto compilado do
   KomboScript carrega para a VM rodar. A matriz densa se grava como bloco
   contíguo, sem tradução; um mapa esparso exigiria formato próprio,
   decisão que voltaria na geração do objeto final.

## O que fica em aberto

Combos com mais estados (envolvendo `+`/`*`, como `Mashing`) e o alfabeto
completo do jogo (se crescer além de 9 símbolos) aumentam `m` e `n` juntos.
A conta se refaz quando o vetor completo de autômatos do jogo existir.
