// Parte 1 — AFN: fecho vazio, movimento por símbolo, simulação e a bateria de
// casos conferidos à mão (tests/casos.txt).

#include <iostream>
#include <string>
#include <vector>

#include "afn.h"
#include "ajuda.h"
#include "bateria.h"
#include "verificar.h"

using namespace komboscript;
using namespace teste;

namespace {

void testarFechoEMovimento() {
    std::cout << "== Fecho vazio e movimento em (a|b)*c ==\n";
    const AfnCombo afn = afnDe("[a | b]* -> c");

    std::vector<Estado> inicio = afn.fechoVazio({afn.estadoInicial()});
    std::sort(inicio.begin(), inicio.end());
    const std::vector<Estado> esperadoInicio = {0, 2, 4, 6, 7, 8};
    verificar(inicio == esperadoInicio, "fecho vazio do estado inicial e {0, 2, 4, 6, 7, 8}");

    const std::vector<Estado> apos_a = afn.mover(inicio, "a");
    // Só o estado de depois da letra: o 5 (saída da alternância) entra pelo fecho vazio.
    const std::vector<Estado> esperadoA = {1};
    verificar(apos_a == esperadoA, "mover(inicio, a) = {1}, sem o fecho vazio");

    std::vector<Estado> fechoDeA = afn.fechoVazio(apos_a);
    std::sort(fechoDeA.begin(), fechoDeA.end());
    const std::vector<Estado> esperadoFechoA = {0, 1, 2, 4, 5, 7, 8};
    verificar(fechoDeA == esperadoFechoA, "fecho de {1} = {0, 1, 2, 4, 5, 7, 8}");

    verificar(afn.mover(inicio, "z").empty(), "mover por simbolo desconhecido da conjunto vazio");
    verificar(afn.mover({}, "a").empty(), "mover a partir do conjunto vazio da conjunto vazio");

    const std::vector<Estado> alcancadoPorC = afn.mover(inicio, "c");
    verificar(alcancadoPorC.size() == 1 && alcancadoPorC[0] == afn.estadoDeAceitacao(),
              "mover(inicio, c) chega so ao estado de aceitacao");

    // Ciclo de transições vazias: o fecho tem de terminar (marcação de visitados).
    const AfnCombo ciclo = afnDe("Jab**");
    const std::vector<Estado> fecho = ciclo.fechoVazio({ciclo.estadoInicial()});
    verificar(!fecho.empty() && fecho.size() <= ciclo.quantidadeDeEstados(), "fecho de Jab** termina sem repetir estados");
}

void testarSimulacao() {
    std::cout << "== Simulacao de (a|b)*c ==\n";
    const AfnCombo afn = afnDe("[a | b]* -> c");
    verificar(afn.aceita({"a", "b", "a", "b", "a", "c"}), "aceita ababac");
    verificar(!afn.aceita({"a", "b", "a", "b"}), "rejeita abab");
    verificar(afn.aceita({"c"}), "aceita c");
    verificar(!afn.aceita({}), "rejeita a cadeia vazia");
    verificar(!afn.aceita({"c", "c"}), "rejeita cc");
    verificar(!afn.aceita({"a", "x", "c"}), "rejeita simbolo fora do alfabeto");

    const std::vector<Simbolo> cadeia = {"a", "b", "c"};
    const SimulacaoAfn simulacao = afn.simular(cadeia);
    std::cout << afn.formatarSimulacao(cadeia, simulacao);
    verificar(simulacao.aceitou, "simulacao de abc aceita");
    verificarIgual(simulacao.passos.size(), std::size_t{4}, "um passo inicial e um por simbolo lido");
    verificar(simulacao.maiorConjuntoAtivo > 1, "o conjunto ativo chegou a ter mais de um estado (nao deterministico)");
}

void testarBateria(const std::string& caminho) {
    std::cout << "== Bateria conferida a mao (" << caminho << ") so pelo AFN ==\n";
    const std::vector<CasoDeTeste> casos = lerCasos(caminho);
    verificar(!casos.empty(), "a bateria deve ter casos (arquivo " + caminho + ")");

    const ResultadoDaBateria resultado = rodarBateria(casos);
    for (const std::string& falha : resultado.falhas) {
        verificar(false, falha);
    }
    std::cout << "  " << resultado.passaram << "/" << resultado.total << " casos passaram\n";
    verificarIgual(resultado.passaram, resultado.total, "todos os casos da bateria passam pelo AFN");
}

}  // namespace

int main(int argc, char** argv) {
    testarFechoEMovimento();
    testarSimulacao();
    testarBateria(caminhoDosCasos(argc, argv));
    return resumo();
}
