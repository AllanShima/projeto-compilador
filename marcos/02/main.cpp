// Demonstração do leitor de expressões do KomboScript.

#include "expressao.h"

#include <iostream>

namespace {

void demonstrar(const std::string& expressao) {
    std::cout << "Expressao: " << expressao << '\n';

    const komboscript::Resultado resultado = komboscript::analisarExpressao(expressao);
    if (!resultado.ok) {
        std::cout << komboscript::formatarErro(expressao, resultado.erro) << '\n';
        return;
    }

    std::cout << "  arvore: " << komboscript::formatarArvore(resultado.arvore) << '\n';
    std::cout << "  nos:    " << komboscript::tamanho(resultado.arvore) << '\n';
}

} // namespace

int main() {
    demonstrar("ChuteFraco -> ChuteFraco+");   // Slide 3
    std::cout << '\n';
    demonstrar("[Jab -> Frente");              // Slide 4
    return 0;
}
