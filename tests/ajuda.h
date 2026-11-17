// ajuda.h — pequenas funções que mais de um teste usa.

#ifndef KOMBOSCRIPT_TESTES_AJUDA_H
#define KOMBOSCRIPT_TESTES_AJUDA_H

#include <algorithm>
#include <string>
#include <vector>

#include "afn.h"
#include "automato.h"
#include "expressao.h"
#include "thompson.h"
#include "verificar.h"

namespace teste {

using namespace komboscript;

// Expressão (texto) -> AFN, pelo caminho que o projeto usa: leitura, árvore,
// Thompson. Falha o teste se a expressão for recusada pela leitura.
inline AfnCombo afnDe(const std::string& expressao) {
    const Resultado leitura = analisarExpressao(expressao);
    verificar(leitura.ok, "a expressao \"" + expressao + "\" deve ser lida");
    return construirThompson(leitura.arvore);
}

// Alfabeto de um caso da bateria: o que a expressão usa mais o que a cadeia
// traz. Assim um símbolo estranho (Chute999) entra no AFD e cai no erro, em
// vez de ser ignorado.
inline std::vector<Simbolo> alfabetoDoCaso(const AfnCombo& afn, const std::vector<Simbolo>& sequencia) {
    std::vector<Simbolo> alfabeto = afn.simbolosUsados();
    for (const Simbolo& s : sequencia) {
        if (std::find(alfabeto.begin(), alfabeto.end(), s) == alfabeto.end()) {
            alfabeto.push_back(s);
        }
    }
    return alfabeto;
}

// Caminho da bateria: primeiro argumento do programa, ou o padrão (o ctest
// roda os testes na raiz do repositório).
inline std::string caminhoDosCasos(const int argc, char** argv) {
    return argc > 1 ? argv[1] : "tests/casos.txt";
}

// "o n-ésimo símbolo a partir do fim é a":  [a|b]* -> a -> [a|b]^(n-1)
inline std::string enesimoDoFim(const std::size_t n) {
    std::string expressao = "[a | b]* -> a";
    for (std::size_t i = 1; i < n; ++i) {
        expressao += " -> [a | b]";
    }
    return expressao;
}

}  // namespace teste

#endif  // KOMBOSCRIPT_TESTES_AJUDA_H
