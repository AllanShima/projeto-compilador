// Parte 2 — minimização por refinamento de partições, comparação das três
// máquinas (AFN, AFD e mínimo) e crescimento de estados.

#include <iostream>
#include <string>
#include <vector>

#include "afd.h"
#include "ajuda.h"
#include "bateria.h"
#include "comparacao.h"
#include "determinizacao.h"
#include "minimizacao.h"
#include "verificar.h"

using namespace komboscript;
using namespace teste;

namespace {

void testarAfnDeExemplo() {
    std::cout << "== (a|b)*c sobre {a, b, c} ==\n";
    const std::vector<Simbolo> alfabeto = {"a", "b", "c"};
    const AfnCombo afn = afnDe("[a | b]* -> c");
    const ResultadoDaDeterminizacao det = determinizar(afn, alfabeto);
    const ResultadoDaMinimizacao min = minimizar(det.afd);

    std::cout << "  antes da minimizacao: " << min.estadosAntes << " estados (erro incluido)"
              << " | depois: " << min.estadosDepois << " estados"
              << " | rodadas de refinamento: " << min.rodadas << '\n';
    std::cout << min.afd.formatarTabela();

    verificarIgual(min.estadosAntes, std::size_t{5}, "antes da minimizacao: 5 estados");
    verificarIgual(min.estadosDepois, std::size_t{3}, "o minimo completo tem 3 estados (inicio, aceita, erro)");
    verificarIgual(min.afd.quantidadeDeEstados(), std::size_t{3}, "o AfdCombo minimo tem 3 posicoes (erro incluido)");

    // q0, q1 e q2 do AFD fazem a mesma coisa: voltam a si mesmos em a e b e vão a aceitação em c.
    verificarIgual(min.classeDe[0], min.classeDe[1], "q0 e q1 caem na mesma classe");
    verificarIgual(min.classeDe[0], min.classeDe[2], "q0 e q2 caem na mesma classe");

    const ComparacaoDasTresMaquinas c = compararMaquinas(afn, det.afd, min.afd, alfabeto, 4);
    std::cout << "  cadeias ate comprimento 4: " << c.cadeias << " | aceitas: AFN " << c.aceitasPeloAfn
              << ", AFD " << c.aceitasPeloAfd << ", minimo " << c.aceitasPeloMinimo << '\n';
    for (const std::string& d : c.divergencias) {
        std::cout << "    divergencia: " << d << '\n';
    }
    verificarIgual(c.cadeias, std::size_t{121}, "121 cadeias ate comprimento 4");
    verificarIgual(c.aceitasPeloAfn, std::size_t{15}, "AFN aceita 15");
    verificarIgual(c.aceitasPeloAfd, std::size_t{15}, "AFD aceita 15");
    verificarIgual(c.aceitasPeloMinimo, std::size_t{15}, "minimo aceita 15");
    verificar(c.concordam(), "as tres maquinas concordam em todas as cadeias");
}

void testarBateria(const std::string& caminho) {
    std::cout << "== Bateria conferida a mao (" << caminho << ") pelas tres maquinas ==\n";
    const std::vector<CasoDeTeste> casos = lerCasos(caminho);
    verificar(!casos.empty(), "a bateria deve ter casos (arquivo " + caminho + ")");

    std::size_t passaram = 0;
    for (const CasoDeTeste& caso : casos) {
        if (!caso.erroDeFormato.empty()) {
            verificar(false, caso.origem + ": " + caso.erroDeFormato);
            continue;
        }
        const AfnCombo afn = afnDe(caso.expressao);
        const ResultadoDaDeterminizacao det = determinizar(afn, alfabetoDoCaso(afn, caso.sequencia));
        const ResultadoDaMinimizacao min = minimizar(det.afd);
        const bool peloAfn = afn.aceita(caso.sequencia);
        const bool peloAfd = det.afd.aceita(caso.sequencia);
        const bool peloMinimo = min.afd.aceita(caso.sequencia);
        const bool ok = peloAfn == caso.esperado && peloAfd == caso.esperado && peloMinimo == caso.esperado;
        passaram += ok;
        verificar(ok, caso.origem + ": /" + caso.expressao + "/ esperado " + (caso.esperado ? "aceita" : "recusa") +
                          " (AFN=" + (peloAfn ? "S" : "N") + " AFD=" + (peloAfd ? "S" : "N") +
                          " min=" + (peloMinimo ? "S" : "N") + ")");
    }
    std::cout << "  " << passaram << "/" << casos.size() << " casos: AFN, AFD e minimo conferem com o papel\n";
}

void testarPadroes() {
    std::cout << "== Outros padroes: AFN -> AFD -> minimo ==\n";
    struct Padrao {
        std::string expressao;
        std::vector<Simbolo> alfabeto;
        std::size_t minimoEsperado;  // completo, com o erro
    };
    const std::vector<Padrao> padroes = {
        {"[a | b]* -> a -> b -> b", {"a", "b"}, 4},  // o clássico: 5 -> 4
        {"a -> b", {"a", "b"}, 4},
        {"a*", {"a", "b"}, 2},
        {"[a | b]*", {"a", "b"}, 1},
        {"a+ -> b", {"a", "b"}, 4},
        {"a?", {"a"}, 3},
    };
    for (const Padrao& p : padroes) {
        const AfnCombo afn = afnDe(p.expressao);
        const ResultadoDaDeterminizacao det = determinizar(afn, p.alfabeto);
        const ResultadoDaMinimizacao min = minimizar(det.afd);
        std::cout << "  " << p.expressao << " : AFN " << afn.quantidadeDeEstados() << " -> AFD "
                  << det.quantidadeDeEstados() << " -> minimo " << min.estadosDepois << '\n';
        verificarIgual(min.estadosDepois, p.minimoEsperado, p.expressao + ": estados do minimo");

        const ComparacaoDasTresMaquinas c = compararMaquinas(afn, det.afd, min.afd, p.alfabeto, 6);
        verificar(c.concordam(), p.expressao + ": AFN, AFD e minimo concordam ate o comprimento 6");

        // Minimizar de novo não pode reduzir mais: o mínimo é ponto fixo.
        const ResultadoDaMinimizacao denovo = minimizar(min.afd);
        verificarIgual(denovo.estadosDepois, min.estadosDepois, p.expressao + ": minimizar o minimo nao muda");
    }
}

// Passos de completar e descartar inacessíveis: um AFD montado à mão, com um
// estado inalcançável e dois estados equivalentes.
void testarAfdComRedundancia() {
    std::cout << "== AFD com inacessivel e equivalentes (montado a mao) ==\n";
    // Linguagem: cadeias sobre {a, b} que terminam em a. q1 e q3 são equivalentes.
    AfdCombo afd({"a", "b"}, 5, 0);
    afd.definirTransicao(0, "a", 1);
    afd.definirTransicao(0, "b", 2);
    afd.definirTransicao(1, "a", 1);
    afd.definirTransicao(1, "b", 2);
    afd.definirTransicao(2, "a", 3);
    afd.definirTransicao(2, "b", 2);
    afd.definirTransicao(3, "a", 3);
    afd.definirTransicao(3, "b", 2);
    afd.definirTransicao(4, "a", 1);  // q4 nunca é alcançado
    afd.marcarAceitacao(1);
    afd.marcarAceitacao(3);
    afd.marcarAceitacao(4);

    const ResultadoDaMinimizacao min = minimizar(afd);
    std::cout << "  antes: " << min.estadosAntes << " estados (erro incluido), alcancaveis: "
              << min.estadosAlcancaveis << ", depois: " << min.estadosDepois << '\n';
    verificarIgual(min.estadosAntes, std::size_t{6}, "6 estados na entrada (5 + erro)");
    verificarIgual(min.estadosAlcancaveis, std::size_t{4}, "4 alcancaveis: q4 e o erro nao sao alcancados");
    verificarIgual(min.estadosDepois, std::size_t{2}, "o minimo tem 2 estados: 'terminou em a' e 'nao'");
    verificarIgual(min.classeDe[4], ResultadoDaMinimizacao::kInacessivel, "q4 foi descartado");
    verificarIgual(min.classeDe[1], min.classeDe[3], "q1 e q3 caem na mesma classe");

    const std::vector<std::vector<Simbolo>> cadeias = {
        {}, {"a"}, {"b"}, {"a", "b"}, {"b", "a"}, {"b", "b", "a"}, {"a", "a", "b", "a"}};
    for (const auto& cadeia : cadeias) {
        verificar(afd.aceita(cadeia) == min.afd.aceita(cadeia), "o minimo reconhece o mesmo que o original");
    }

    // O AFD do marco 03, montado à mão, já deve ser mínimo.
    const ResultadoDaMinimizacao mh = minimizar(montarHadouken());
    std::cout << "  Hadouken (montado a mao): " << mh.estadosAntes << " -> " << mh.estadosDepois << " estados\n";
    verificarIgual(mh.estadosDepois, mh.estadosAntes, "o Hadouken montado a mao ja era minimo");
}

void testarCrescimento() {
    std::cout << "== Crescimento: o n-esimo simbolo a partir do fim e 'a' ==\n";
    std::cout << "  padrao: " << enesimoDoFim(3) << "   (n = 3)\n";
    std::cout << "    n | AFN estados | AFD estados | minimo estados | 2^n\n";
    std::cout << "    --+-------------+-------------+----------------+--------\n";

    auto coluna = [](const std::size_t valor, const std::size_t largura) {
        const std::string t = std::to_string(valor);
        return std::string(largura > t.size() ? largura - t.size() : 0, ' ') + t;
    };

    for (std::size_t n = 1; n <= 12; ++n) {
        const AfnCombo afn = afnDe(enesimoDoFim(n));
        const ResultadoDaDeterminizacao det = determinizar(afn, {"a", "b"});
        const ResultadoDaMinimizacao min = minimizar(det.afd);
        const std::size_t potencia = std::size_t{1} << n;

        std::cout << "    " << coluna(n, 2) << " |" << coluna(afn.quantidadeDeEstados(), 12) << " |"
                  << coluna(det.quantidadeDeEstados(), 12) << " |" << coluna(min.estadosDepois, 15) << " |"
                  << coluna(potencia, 7) << '\n';

        // O AFD tem 2^n + 1; o subconjunto inicial de Thompson é equivalente ao
        // estado "só b" e a minimização o une a esse, restando exatamente 2^n.
        verificarIgual(det.quantidadeDeEstados(), potencia + 1, "n=" + std::to_string(n) + ": AFD tem 2^n + 1 estados");
        verificarIgual(min.estadosDepois, potencia, "n=" + std::to_string(n) + ": o minimo tem 2^n estados");

        if (n <= 8) {
            const ComparacaoDasTresMaquinas c = compararMaquinas(afn, det.afd, min.afd, {"a", "b"}, 10);
            verificar(c.concordam(), "n=" + std::to_string(n) + ": as tres maquinas concordam ate o comprimento 10");
        }
    }
    std::cout << "  O AFN cresce linearmente (6n + 4 estados); o AFD, exponencialmente (2^n + 1). A\n"
                 "  minimizacao so elimina o +1 (o subconjunto inicial de Thompson): os 2^n estados\n"
                 "  restantes sao todos distinguiveis (cada um lembra as n ultimas letras), logo 2^n\n"
                 "  e o minimo possivel.\n";
}

}  // namespace

int main(int argc, char** argv) {
    testarAfnDeExemplo();
    testarBateria(caminhoDosCasos(argc, argv));
    testarPadroes();
    testarAfdComRedundancia();
    testarCrescimento();
    return resumo();
}
