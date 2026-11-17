// Parte 2 — determinização por subconjuntos: AFN -> AFD.
// O conjunto vazio é o estado de erro; cada estado do AFD guarda o conjunto de
// estados do AFN que representa.

#include <iostream>
#include <string>
#include <vector>

#include "afd.h"
#include "ajuda.h"
#include "bateria.h"
#include "comparacao.h"
#include "determinizacao.h"
#include "verificar.h"

using namespace komboscript;
using namespace teste;

namespace {

void testarAfnDeExemplo() {
    std::cout << "== (a|b)*c sobre {a, b, c} ==\n";
    const std::vector<Simbolo> alfabeto = {"a", "b", "c"};
    const AfnCombo afn = afnDe("[a | b]* -> c");
    const ResultadoDaDeterminizacao det = determinizar(afn, alfabeto);

    std::cout << "  AFN: " << afn.quantidadeDeEstados() << " estados\n";
    std::cout << "  AFD (subconjuntos): " << det.quantidadeDeEstados() << " estados\n";
    for (std::size_t i = 0; i < det.conjuntos.size(); ++i) {
        std::cout << "    q" << i << " = " << formatarConjunto(det.conjuntos[i]) << '\n';
    }

    verificarIgual(det.quantidadeDeEstados(), std::size_t{5}, "AFD tem 5 estados, contado o vazio (erro)");
    verificar(det.vazioAlcancavel, "o conjunto vazio e alcancavel (c depois de c)");
    verificarIgual(det.conjuntos.size(), std::size_t{4}, "4 conjuntos nao vazios");

    // O estado inicial é o fecho vazio do inicial do AFN.
    const std::vector<Estado> inicio = {0, 2, 4, 6, 7, 8};
    verificar(det.conjuntos[0] == inicio, "q0 = fecho vazio do inicial = {0, 2, 4, 6, 7, 8}");

    // Todo estado de aceitação do AFD contém o estado de aceitação do AFN, e só eles.
    for (Estado e = 0; e < det.conjuntos.size(); ++e) {
        const bool contem = std::find(det.conjuntos[e].begin(), det.conjuntos[e].end(), afn.estadoDeAceitacao()) !=
                            det.conjuntos[e].end();
        verificar(det.afd.ehDeAceitacao(e) == contem, "q" + std::to_string(e) + " aceita se e so se contem o estado final do AFN");
    }

    // A função de transição é total: um símbolo fora do alfabeto cai no erro.
    verificarIgual(det.afd.transicao(0, "x"), det.afd.estadoDeErro(), "simbolo fora do alfabeto cai no erro");

    // AFN x AFD em todas as cadeias até o comprimento 4. (A comparação recebe
    // três máquinas; aqui o AFD ocupa o lugar das duas últimas.)
    const ComparacaoDasTresMaquinas c = compararMaquinas(afn, det.afd, det.afd, alfabeto, 4);
    verificarIgual(c.cadeias, std::size_t{121}, "121 cadeias ate comprimento 4");
    verificarIgual(c.aceitasPeloAfn, std::size_t{15}, "AFN aceita 15");
    verificarIgual(c.aceitasPeloAfd, std::size_t{15}, "AFD aceita 15");
    verificar(c.concordam(), "AFN e AFD concordam em todas as cadeias");
}

void testarVazioNaoAlcancado() {
    std::cout << "== O vazio so conta quando e alcancado ==\n";
    // [a|b]* sempre tem transição: o vazio nunca aparece.
    const ResultadoDaDeterminizacao sempre = determinizar(afnDe("[a | b]*"), {"a", "b"});
    verificar(!sempre.vazioAlcancavel, "[a|b]* sobre {a,b}: o vazio nao e alcancado");
    // Um símbolo do alfabeto que a expressão não usa leva ao vazio.
    const ResultadoDaDeterminizacao com = determinizar(afnDe("[a | b]*"), {"a", "b", "c"});
    verificar(com.vazioAlcancavel, "[a|b]* sobre {a,b,c}: c leva ao vazio");
    verificarIgual(com.quantidadeDeEstados(), sempre.quantidadeDeEstados() + 1, "o vazio acrescenta um estado");
}

void testarBateria(const std::string& caminho) {
    std::cout << "== Bateria conferida a mao (" << caminho << ") pelo AFD ==\n";
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
        const bool obtido = det.afd.aceita(caso.sequencia);
        passaram += obtido == caso.esperado;
        verificar(obtido == caso.esperado, caso.origem + ": /" + caso.expressao + "/ esperado " +
                                               (caso.esperado ? "aceita" : "recusa"));
    }
    std::cout << "  " << passaram << "/" << casos.size() << " casos passaram pelo AFD\n";
}

void testarPadroes() {
    std::cout << "== Outros padroes: AFN -> AFD ==\n";
    struct Padrao {
        std::string expressao;
        std::vector<Simbolo> alfabeto;
        std::size_t afdEsperado;  // com o vazio, se alcançável
    };
    const std::vector<Padrao> padroes = {
        {"[a | b]* -> a -> b -> b", {"a", "b"}, 5},  // o clássico: 5 estados, sem vazio
        {"a -> b", {"a", "b"}, 4},
        {"a*", {"a", "b"}, 3},
        // O subconjunto inicial de Thompson (sem os estados "apos a letra") nunca se repete.
        {"[a | b]*", {"a", "b"}, 3},
        {"a+ -> b", {"a", "b"}, 5},
        {"a?", {"a"}, 3},
    };
    for (const Padrao& p : padroes) {
        const AfnCombo afn = afnDe(p.expressao);
        const ResultadoDaDeterminizacao det = determinizar(afn, p.alfabeto);
        std::cout << "  " << p.expressao << " : AFN " << afn.quantidadeDeEstados() << " -> AFD "
                  << det.quantidadeDeEstados() << '\n';
        verificarIgual(det.quantidadeDeEstados(), p.afdEsperado, p.expressao + ": estados do AFD");
        const ComparacaoDasTresMaquinas c = compararMaquinas(afn, det.afd, det.afd, p.alfabeto, 6);
        verificar(c.concordam(), p.expressao + ": AFN e AFD concordam ate o comprimento 6");
    }
}

// Um AFD montado à mão (marco 03) também precisa continuar reconhecendo o que
// reconhecia: é a rede de segurança de `AfdCombo`.
void testarAfdMontadoAMao() {
    std::cout << "== AFD do Hadouken, montado a mao ==\n";
    const AfdCombo hadouken = montarHadouken();
    struct Caso {
        std::string nome;
        std::vector<Simbolo> sequencia;
        bool aceita;
    };
    const std::vector<Caso> casos = {
        {"Hadouken direto", {"Baixo", "Frente", "Jab"}, true},
        {"Hadouken com rolamento segurado", {"Baixo", "Baixo", "Frente", "Jab"}, true},
        {"Rolamento para tras nao serve", {"Baixo", "Tras", "Jab"}, false},
        {"Falta o Jab final", {"Baixo", "Frente"}, false},
        {"Sequencia vazia", {}, false},
        {"Simbolo fora do alfabeto do jogo", {"Baixo", "Frente", "Chute999"}, false},
    };
    for (const Caso& caso : casos) {
        verificar(hadouken.aceita(caso.sequencia) == caso.aceita, "Hadouken: " + caso.nome);
    }
}

void testarCrescimento() {
    std::cout << "== Crescimento: o n-esimo simbolo a partir do fim e 'a' ==\n";
    std::size_t anterior = 0;
    for (std::size_t n = 1; n <= 10; ++n) {
        const AfnCombo afn = afnDe(enesimoDoFim(n));
        const ResultadoDaDeterminizacao det = determinizar(afn, {"a", "b"});
        const std::size_t potencia = std::size_t{1} << n;

        verificarIgual(afn.quantidadeDeEstados(), 6 * n + 4, "n=" + std::to_string(n) + ": AFN tem 6n + 4 estados");
        // 2^n subconjuntos "janela das n últimas letras" + o subconjunto inicial.
        verificarIgual(det.quantidadeDeEstados(), potencia + 1, "n=" + std::to_string(n) + ": AFD tem 2^n + 1 estados");
        verificar(!det.vazioAlcancavel, "n=" + std::to_string(n) + ": o vazio nunca e alcancado");
        if (anterior != 0) {
            verificarIgual(det.quantidadeDeEstados() - 1, 2 * (anterior - 1), "n=" + std::to_string(n) + ": 2^n dobra a cada n");
        }
        anterior = det.quantidadeDeEstados();

        if (n <= 6) {
            const ComparacaoDasTresMaquinas c = compararMaquinas(afn, det.afd, det.afd, {"a", "b"}, 8);
            verificar(c.concordam(), "n=" + std::to_string(n) + ": AFN e AFD concordam ate o comprimento 8");
        }
    }
    std::cout << "  AFN: 6n + 4 estados; AFD: 2^n + 1 (tabela completa em test_minimizacao)\n";
}

}  // namespace

int main(int argc, char** argv) {
    testarAfnDeExemplo();
    testarVazioNaoAlcancado();
    testarBateria(caminhoDosCasos(argc, argv));
    testarPadroes();
    testarAfdMontadoAMao();
    testarCrescimento();
    return resumo();
}
