// Demonstração e testes dos marcos 04 e 05: Thompson, simulação do AFN,
// determinização, minimização, comparação das três máquinas e crescimento
// de estados. Sai com código 1 se qualquer verificação falhar.
//
// Uso: main_afd_min [caminho/para/casos.txt]   (padrão: marcos/04/casos.txt)

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#include "afd.h"
#include "afn.h"
#include "comparacao.h"
#include "determinizacao.h"
#include "expressao.h"
#include "minimizacao.h"

using namespace komboscript;

namespace {

std::size_t verificacoes = 0;
std::size_t falhas = 0;

void verificar(const bool condicao, const std::string& descricao) {
    ++verificacoes;
    if (!condicao) {
        ++falhas;
        std::cout << "  FALHOU: " << descricao << '\n';
    }
}

template <typename T>
void verificarIgual(const T& obtido, const T& esperado, const std::string& descricao) {
    ++verificacoes;
    if (!(obtido == esperado)) {
        ++falhas;
        std::cout << "  FALHOU: " << descricao << " — esperado " << esperado << ", obtido " << obtido << '\n';
    }
}

AfnCombo afnDe(const std::string& expressao) {
    const Resultado leitura = analisarExpressao(expressao);
    verificar(leitura.ok, "a expressao \"" + expressao + "\" deve ser lida");
    return construirThompson(leitura.arvore);
}

// --- Parte 1: Thompson -------------------------------------------------

struct ContagemEsperada {
    std::string expressao;
    std::size_t estados;
    std::size_t transicoes;
    std::size_t vazias;
};

void testarThompson() {
    std::cout << "== Thompson: contagens (estados / transicoes / vazias) ==\n";
    // Tabela "Testes de Thompson" do guia, na notação do KomboScript.
    // `a.c` do guia usa coringa, que a linguagem não tem: fica de fora.
    const std::vector<ContagemEsperada> tabela = {
        {"a", 2, 1, 0},
        {"a -> b", 4, 3, 1},
        {"a | b", 6, 6, 4},
        {"a*", 4, 5, 4},
        {"[a | b]* -> c", 10, 12, 9},
        {"[a -> b -> c]*", 8, 9, 6},
        // Reduções do açúcar: a+ = a -> a*, a? = a | vazio.
        // a+ = concat(a, fecho(a)): 2 + 4 estados; 1 + 5 transicoes + a vazia da concatenacao.
        {"a+", 6, 7, 5},
        // a? = alt(a, vazio): o ramo vazio ja e uma transicao vazia, alem das 4 da alternancia.
        {"a?", 6, 6, 5},
    };
    for (const ContagemEsperada& linha : tabela) {
        const AfnCombo afn = afnDe(linha.expressao);
        std::cout << "  " << linha.expressao << " : " << afn.quantidadeDeEstados() << " / "
                  << afn.quantidadeDeTransicoes() << " / " << afn.quantidadeDeTransicoesVazias() << '\n';
        verificarIgual(afn.quantidadeDeEstados(), linha.estados, linha.expressao + ": estados");
        verificarIgual(afn.quantidadeDeTransicoes(), linha.transicoes, linha.expressao + ": transicoes");
        verificarIgual(afn.quantidadeDeTransicoesVazias(), linha.vazias, linha.expressao + ": vazias");
    }

    const AfnCombo afn = afnDe("[a | b]* -> c");
    verificar(afn.aceita({"a", "b", "a", "b", "a", "c"}), "(a|b)*c aceita ababac");
    verificar(!afn.aceita({"a", "b", "a", "b"}), "(a|b)*c rejeita abab");
    verificar(afn.aceita({"c"}), "(a|b)*c aceita c");
    verificar(!afn.aceita({}), "(a|b)*c rejeita a cadeia vazia");
}

// --- Parte 1: a bateria conferida à mão, agora pelas três máquinas -------

void testarBateriaDeCasos(const std::string& caminho) {
    std::cout << "== Bateria conferida a mao (" << caminho << ") nas tres maquinas ==\n";
    const std::vector<CasoDeTeste> casos = lerCasos(caminho);
    verificar(!casos.empty(), "a bateria deve ter casos (arquivo " + caminho + ")");

    std::size_t passaram = 0;
    for (const CasoDeTeste& caso : casos) {
        if (!caso.erroDeFormato.empty()) {
            verificar(false, caso.origem + ": " + caso.erroDeFormato);
            continue;
        }
        const Resultado leitura = analisarExpressao(caso.expressao);
        if (!leitura.ok) {
            verificar(false, caso.origem + ": expressao recusada pela leitura");
            continue;
        }
        const AfnCombo afn = construirThompson(leitura.arvore);

        // Alfabeto: o que a expressao usa mais o que a cadeia traz. Assim um
        // simbolo estranho (Chute999) entra no AFD e cai no erro, em vez de
        // ser ignorado.
        std::vector<Simbolo> alfabeto = afn.simbolosUsados();
        for (const Simbolo& s : caso.sequencia) {
            if (std::find(alfabeto.begin(), alfabeto.end(), s) == alfabeto.end()) {
                alfabeto.push_back(s);
            }
        }
        const ResultadoDaDeterminizacao det = determinizar(afn, alfabeto);
        const ResultadoDaMinimizacao min = minimizar(det.afd);

        const bool peloAfn = afn.aceita(caso.sequencia);
        const bool peloAfd = det.afd.aceita(caso.sequencia);
        const bool peloMinimo = min.afd.aceita(caso.sequencia);
        const bool ok = peloAfn == caso.esperado && peloAfd == caso.esperado && peloMinimo == caso.esperado;
        if (ok) {
            ++passaram;
        }
        verificar(ok, caso.origem + ": /" + caso.expressao + "/ esperado " +
                          (caso.esperado ? "aceita" : "recusa") + " (AFN=" + (peloAfn ? "S" : "N") +
                          " AFD=" + (peloAfd ? "S" : "N") + " min=" + (peloMinimo ? "S" : "N") + ")");
    }
    std::cout << "  " << passaram << "/" << casos.size() << " casos: AFN, AFD e minimo conferem com o papel\n";
}

// --- Parte 2: (a|b)*c ----------------------------------------------------

void testarDeterminizacaoEMinimizacao() {
    std::cout << "== (a|b)*c sobre {a, b, c} ==\n";
    const std::vector<Simbolo> alfabeto = {"a", "b", "c"};
    const AfnCombo afn = afnDe("[a | b]* -> c");
    const ResultadoDaDeterminizacao det = determinizar(afn, alfabeto);
    const ResultadoDaMinimizacao min = minimizar(det.afd);

    std::cout << "  AFN: " << afn.quantidadeDeEstados() << " estados\n";
    std::cout << "  AFD (subconjuntos): " << det.quantidadeDeEstados() << " estados\n";
    for (std::size_t i = 0; i < det.conjuntos.size(); ++i) {
        std::cout << "    q" << i << " = " << formatarConjunto(det.conjuntos[i]) << '\n';
    }
    std::cout << "  antes da minimizacao: " << min.estadosAntes << " estados (erro incluido)"
              << " | depois: " << min.estadosDepois << " estados"
              << " | rodadas de refinamento: " << min.rodadas << '\n';
    std::cout << min.afd.formatarTabela();

    verificarIgual(det.quantidadeDeEstados(), std::size_t{5}, "AFD tem 5 estados, contado o vazio (erro)");
    verificar(det.vazioAlcancavel, "o conjunto vazio e alcancavel (c leva a {}, e c-depois-de-c tambem)");
    verificarIgual(min.estadosAntes, std::size_t{5}, "antes da minimizacao: 5 estados");
    verificarIgual(min.estadosDepois, std::size_t{3}, "o minimo completo tem 3 estados (inicio, aceita, erro)");
    verificarIgual(min.afd.quantidadeDeEstados(), std::size_t{3}, "o AfdCombo minimo tem 3 posicoes (erro incluido)");

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

// --- Parte 2: outros padroes --------------------------------------------

void testarPadroes() {
    std::cout << "== Outros padroes: estados do AFN -> AFD -> minimo ==\n";
    struct Padrao {
        std::string expressao;
        std::vector<Simbolo> alfabeto;
        std::size_t afdEsperado;     // com o vazio, se alcancavel
        std::size_t minimoEsperado;  // completo, com o erro
    };
    const std::vector<Padrao> padroes = {
        {"[a | b]* -> a -> b -> b", {"a", "b"}, 5, 4},  // classico: nao ha vazio
        {"a -> b", {"a", "b"}, 4, 4},
        {"a*", {"a", "b"}, 3, 2},
        // O subconjunto inicial de Thompson (sem os estados "apos a letra") nunca
        // se repete: 3 estados no AFD, que a minimizacao reduz a 1.
        {"[a | b]*", {"a", "b"}, 3, 1},
        {"a+ -> b", {"a", "b"}, 5, 4},
        {"a?", {"a"}, 3, 3},
    };
    for (const Padrao& p : padroes) {
        const AfnCombo afn = afnDe(p.expressao);
        const ResultadoDaDeterminizacao det = determinizar(afn, p.alfabeto);
        const ResultadoDaMinimizacao min = minimizar(det.afd);
        std::cout << "  " << p.expressao << " : AFN " << afn.quantidadeDeEstados() << " -> AFD "
                  << det.quantidadeDeEstados() << " -> minimo " << min.estadosDepois << '\n';
        verificarIgual(det.quantidadeDeEstados(), p.afdEsperado, p.expressao + ": estados do AFD");
        verificarIgual(min.estadosDepois, p.minimoEsperado, p.expressao + ": estados do minimo");

        const ComparacaoDasTresMaquinas c = compararMaquinas(afn, det.afd, min.afd, p.alfabeto, 6);
        verificar(c.concordam(), p.expressao + ": AFN, AFD e minimo concordam ate o comprimento 6");

        // Minimizar de novo nao pode reduzir mais: o minimo e ponto fixo.
        const ResultadoDaMinimizacao denovo = minimizar(min.afd);
        verificarIgual(denovo.estadosDepois, min.estadosDepois, p.expressao + ": minimizar o minimo nao muda");
    }
}

// Passos 18-19 do roteiro: um AFD com estado inacessivel e dois estados
// equivalentes, montado a mao, para provar que a minimizacao os remove.
void testarAfdComRedundancia() {
    std::cout << "== AFD com inacessivel e equivalentes (montado a mao) ==\n";
    // Linguagem: cadeias sobre {a, b} que terminam em a.
    // q0 --a--> q1, q0 --b--> q2 ; q1 e q2 sao equivalentes aos pares (q1 ~ q3).
    AfdCombo afd({"a", "b"}, 5, 0);
    afd.definirTransicao(0, "a", 1);
    afd.definirTransicao(0, "b", 2);
    afd.definirTransicao(1, "a", 1);
    afd.definirTransicao(1, "b", 2);
    afd.definirTransicao(2, "a", 3);  // q3 e q1 sao equivalentes
    afd.definirTransicao(2, "b", 2);
    afd.definirTransicao(3, "a", 3);
    afd.definirTransicao(3, "b", 2);
    afd.definirTransicao(4, "a", 1);  // q4 nunca e alcancado
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

    // O AFD do marco 03, montado a mao, ja deve ser minimo.
    const AfdCombo hadouken = montarHadouken();
    const ResultadoDaMinimizacao mh = minimizar(hadouken);
    std::cout << "  Hadouken (marco 03): " << mh.estadosAntes << " -> " << mh.estadosDepois << " estados\n";
    verificarIgual(mh.estadosDepois, mh.estadosAntes, "o Hadouken do marco 03 ja era minimo");
}

// --- Parte 2: explosão de estados ----------------------------------------

// "o n-ésimo símbolo a partir do fim é a":  [a|b]* -> a -> [a|b]^(n-1)
std::string enesimoDoFim(const std::size_t n) {
    std::string expressao = "[a | b]* -> a";
    for (std::size_t i = 1; i < n; ++i) {
        expressao += " -> [a | b]";
    }
    return expressao;
}

void testarExplosao() {
    std::cout << "== Explosao de estados: o n-esimo simbolo a partir do fim e 'a' ==\n";
    std::cout << "  padrao: " << enesimoDoFim(3) << "   (n = 3)\n";
    std::cout << "    n | AFN estados | AFD estados | minimo estados | 2^n\n";
    std::cout << "    --+-------------+-------------+----------------+--------\n";
    // AFD = 2^n + 1 (ver abaixo); minimo = 2^n.

    std::size_t anterior = 0;
    for (std::size_t n = 1; n <= 12; ++n) {
        const AfnCombo afn = afnDe(enesimoDoFim(n));
        const ResultadoDaDeterminizacao det = determinizar(afn, {"a", "b"});
        const ResultadoDaMinimizacao min = minimizar(det.afd);
        const std::size_t potencia = std::size_t{1} << n;

        std::string linha = "    ";
        auto coluna = [&](const std::size_t valor, const std::size_t largura) {
            std::string t = std::to_string(valor);
            return std::string(largura > t.size() ? largura - t.size() : 0, ' ') + t;
        };
        linha += coluna(n, 2) + " |" + coluna(afn.quantidadeDeEstados(), 12) + " |" +
                 coluna(det.quantidadeDeEstados(), 12) + " |" + coluna(min.estadosDepois, 15) + " |" +
                 coluna(potencia, 7);
        std::cout << linha << '\n';

        // O AFD tem 2^n + 1: os 2^n subconjuntos "janela das n ultimas letras" e o
        // subconjunto inicial de Thompson, que nunca e revisitado. Ele e equivalente
        // ao estado "so b" e a minimizacao o une a esse, restando exatamente 2^n.
        verificarIgual(det.quantidadeDeEstados(), potencia + 1, "n=" + std::to_string(n) + ": AFD tem 2^n + 1 estados");
        verificarIgual(min.estadosDepois, potencia, "n=" + std::to_string(n) + ": o minimo tem 2^n estados");
        if (anterior != 0) {
            verificarIgual(det.quantidadeDeEstados() - 1, 2 * (anterior - 1), "n=" + std::to_string(n) + ": 2^n dobra a cada n");
        }
        anterior = det.quantidadeDeEstados();

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
    const std::string caminho = argc > 1 ? argv[1] : "marcos/04/casos.txt";

    testarThompson();
    testarBateriaDeCasos(caminho);
    testarDeterminizacaoEMinimizacao();
    testarPadroes();
    testarAfdComRedundancia();
    testarExplosao();

    std::cout << "\n" << (verificacoes - falhas) << "/" << verificacoes << " verificacoes passaram.\n";
    return falhas == 0 ? 0 : 1;
}
