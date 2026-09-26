#include <iostream>

#include "afd.h"

using namespace komboscript;

int main() {
    const AfdCombo hadouken = montarHadouken();

    std::cout << "=== Tabela de transicoes (Hadouken) ===\n";
    std::cout << hadouken.formatarTabela() << "\n";

    std::cout << "=== A conta ===\n";
    std::cout << "estados uteis:        " << (hadouken.quantidadeDeEstados() - 1) << "\n";
    std::cout << "alfabeto (m):         " << hadouken.tamanhoDoAlfabeto() << "\n";
    std::cout << "posicoes na tabela:   " << hadouken.quantidadeDeEstados() * hadouken.tamanhoDoAlfabeto() << "\n";
    std::cout << "bytes da tabela:      " << hadouken.bytesDaTabela() << "\n";
    std::cout << "transicoes != erro:   " << hadouken.transicoesDefinidas() << "\n\n";

    std::cout << "=== Cadeia aceita ===\n";
    const std::vector<Simbolo> aceita = {"Baixo", "Frente", "Jab"};
    std::cout << hadouken.formatarExecucao(aceita, hadouken.executar(aceita)) << "\n";

    std::cout << "=== Cadeia recusada (simbolo valido, transicao inexistente) ===\n";
    const std::vector<Simbolo> recusada = {"Baixo", "Tras", "Jab"};
    std::cout << hadouken.formatarExecucao(recusada, hadouken.executar(recusada)) << "\n";

    return 0;
}
