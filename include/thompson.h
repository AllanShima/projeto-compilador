// thompson.h — construção de Thompson: árvore da expressão -> AFN.
//
// Recebe a árvore que `analisarExpressao` produz (já reduzida ao núcleo:
// concatenação, alternância, fecho, identificador e cadeia vazia), nunca o
// texto da expressão. Cada tipo de nó vira um bloco com uma entrada e uma
// saída; os blocos se ligam por transições vazias.

#ifndef KOMBOSCRIPT_THOMPSON_H
#define KOMBOSCRIPT_THOMPSON_H

#include "afn.h"
#include "automato.h"
#include "expressao.h"  // Arvore

namespace komboscript {

// Bloco com uma entrada e uma saída.
struct Bloco {
    Estado entrada = 0;
    Estado saida = 0;
};

AfnCombo construirThompson(const Arvore& arvore);

}  // namespace komboscript

#endif  // KOMBOSCRIPT_THOMPSON_H
