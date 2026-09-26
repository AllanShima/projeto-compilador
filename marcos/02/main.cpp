// Demonstração do leitor de expressões do KomboScript.

#include "expressao.h"

#include <iostream>

namespace {

void demonstrarValida(const std::string& expressao) {
    std::cout << "Expressao valida: " << expressao << '\n';
    const komboscript::Resultado resultado = komboscript::analisarExpressao(expressao);
    if (!resultado.ok) {
        std::cout << "  ERRO INESPERADO: "
                   << komboscript::formatarErro(expressao, resultado.erro) << '\n';
        return;
    }
    std::cout << "  arvore: " << komboscript::formatarArvore(resultado.arvore) << '\n';
    std::cout << "  nos:    " << komboscript::tamanho(resultado.arvore) << '\n';
}

void demonstrarMalformada(const std::string& expressao) {
    std::cout << "Expressao malformada: " << expressao << '\n';
    const komboscript::Resultado resultado = komboscript::analisarExpressao(expressao);
    if (resultado.ok) {
        std::cout << "  INESPERADO: a expressao foi aceita, e nao deveria ser.\n";
        return;
    }
    std::cout << komboscript::formatarErro(expressao, resultado.erro) << '\n';
}

}  // namespace

int main() {
    // Slide 3 — expressao real do combo Mashing.
    demonstrarValida("ChuteFraco -> ChuteFraco+");

    std::cout << '\n';

    // Slide 4 — grupo aberto que nunca fecha.
    demonstrarMalformada("[Jab -> Frente");

    return 0;
}