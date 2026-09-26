#include <iostream>

#include "afd.h"

using namespace komboscript;

// Um caso de teste é uma sequência de inputs e o veredito esperado. O nome
// diz qual defeito o caso está ali para apanhar, não só o que ele testa.
struct CasoDeTeste {
    std::string nome;
    std::vector<Simbolo> sequencia;
    bool esperaAceitar;
};

std::vector<CasoDeTeste> casosDeTeste() {
    return {
        // Grupo 1 — o caminho feliz precisa continuar aceitando, inclusive
        // na variação em que o jogador segura o rolamento (Baixo repetido).
        {"Hadouken direto", {"Baixo", "Frente", "Jab"}, true},
        {"Hadouken com rolamento segurado", {"Baixo", "Baixo", "Frente", "Jab"}, true},

        // Grupo 2 — símbolo válido do jogo, mas sem transição prevista
        // naquele estado (rolar para o lado errado).
        {"Rolamento para tras nao serve", {"Baixo", "Tras", "Jab"}, false},

        // Grupo 3 — a cadeia inteira é consumida sem nunca cair no erro, e
        // ainda assim é recusada por parar fora de F. É o caso que a
        // implementação mais esquece.
        {"Falta o Jab final", {"Baixo", "Frente"}, false},
        {"Sequencia vazia", {}, false},

        // Grupo 4 — o input nem pertence ao vocabulário do jogo.
        {"Simbolo fora do alfabeto do jogo", {"Baixo", "Frente", "Chute999"}, false},
    };
}

int main() {
    const AfdCombo hadouken = montarHadouken();
    const std::vector<CasoDeTeste> casos = casosDeTeste();

    std::size_t recusasEsperadas = 0;
    std::size_t acertos = 0;
    Execucao ultimaExecucao;
    std::vector<Simbolo> ultimaSequencia;

    for (const CasoDeTeste& caso : casos) {
        const Execucao execucao = hadouken.executar(caso.sequencia);
        const bool bateuOEsperado = execucao.aceitou == caso.esperaAceitar;

        if (!caso.esperaAceitar) ++recusasEsperadas;
        if (bateuOEsperado) ++acertos;

        std::cout << (bateuOEsperado ? "[OK] " : "[FALHOU] ") << caso.nome << "\n";
        std::cout << hadouken.formatarExecucao(caso.sequencia, execucao) << "\n";

        ultimaExecucao = execucao;
        ultimaSequencia = caso.sequencia;
    }

    return 0;
}
