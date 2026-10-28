// Demonstração e teste do marco 04.
// Uso: main_afn [caminho/para/casos.txt]   (padrão: marcos/04/casos.txt)
// Sai com código 1 se algum caso falhar.

#include <iostream>
#include <string>
#include <vector>

#include "afn.h"

using namespace komboscript;

namespace {

void demonstrar(const std::string& expressao, const std::vector<Simbolo>& sequencia) {
    std::cout << "Expressao: " << expressao << '\n';
    const Resultado leitura = analisarExpressao(expressao);
    if (!leitura.ok) {
        std::cout << formatarErro(expressao, leitura.erro) << '\n';
        return;
    }
    const AfnCombo afn = construirThompson(leitura.arvore);
    std::cout << "  arvore: " << formatarArvore(leitura.arvore) << '\n';
    std::cout << "  afn:    " << afn.quantidadeDeEstados() << " estados, "
              << afn.quantidadeDeTransicoes() << " transicoes ("
              << afn.quantidadeDeTransicoesVazias() << " vazias)\n";
    std::cout << afn.formatarSimulacao(sequencia, afn.simular(sequencia)) << '\n';
}

}  // namespace

int main(int argc, char** argv) {
    demonstrar("Baixo -> Frente -> Jab", {"Baixo", "Frente", "Jab"});
    demonstrar("Baixo | Tras", {"Tras"});
    demonstrar("[Baixo | Tras]* -> SocoForte", {"Baixo", "Tras", "SocoForte"});
    demonstrar("[Baixo | Tras]* -> SocoForte", {"Baixo", "Frente", "SocoForte"});

    const std::string caminho = argc > 1 ? argv[1] : "marcos/04/casos.txt";
    const std::vector<CasoDeTeste> casos = lerCasos(caminho);
    if (casos.empty()) {
        std::cout << "Bateria: nenhum caso lido de " << caminho << " — reprovado.\n";
        return 1;
    }

    const ResultadoDaBateria resultado = rodarBateria(casos);
    for (const std::string& falha : resultado.falhas) {
        std::cout << "FALHOU " << falha << '\n';
    }
    std::cout << "Bateria conferida a mao: " << resultado.passaram << "/" << resultado.total
              << " casos passaram.\n";
    return resultado.passaram == resultado.total ? 0 : 1;
}
