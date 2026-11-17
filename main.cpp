// KomboScript — do padrão ao AFD mínimo.
//
// Uso:
//   komboscript "<expressao>" [simbolo ...]  < cadeias.txt
//
// Lê a expressão (notação do KomboScript: `->` concatena, `|` alterna,
// `*` `+` `?` repetem, `[ ]` agrupa), constrói o AFN de Thompson, determiniza,
// minimiza e imprime o número de estados de cada máquina. Os símbolos depois
// da expressão formam o alfabeto do AFD; sem eles, vale o que a expressão usa.
//
// Cada linha lida da entrada padrão é uma cadeia (símbolos separados por
// espaço; linha vazia = cadeia vazia); o programa diz o que as três máquinas
// respondem.
//
// Exemplo:
//   echo "a b a b a c" | komboscript "[a | b]* -> c" a b c

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "afd.h"
#include "afn.h"
#include "determinizacao.h"
#include "expressao.h"
#include "minimizacao.h"
#include "thompson.h"

using namespace komboscript;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "uso: " << argv[0] << " \"<expressao>\" [simbolo ...]  < cadeias.txt\n";
        return 2;
    }
    const std::string expressao = argv[1];

    const Resultado leitura = analisarExpressao(expressao);
    if (!leitura.ok) {
        std::cout << formatarErro(expressao, leitura.erro) << '\n';
        return 1;
    }

    const AfnCombo afn = construirThompson(leitura.arvore);
    std::vector<Simbolo> alfabeto;
    for (int i = 2; i < argc; ++i) {
        alfabeto.push_back(argv[i]);
    }
    if (alfabeto.empty()) {
        alfabeto = afn.simbolosUsados();
    }
    const ResultadoDaDeterminizacao det = determinizar(afn, alfabeto);
    const ResultadoDaMinimizacao min = minimizar(det.afd);

    std::cout << "Expressao: " << expressao << '\n';
    std::cout << "  arvore: " << formatarArvore(leitura.arvore) << " (" << tamanho(leitura.arvore) << " nos)\n";
    std::cout << "  AFN de Thompson: " << afn.quantidadeDeEstados() << " estados, " << afn.quantidadeDeTransicoes()
              << " transicoes (" << afn.quantidadeDeTransicoesVazias() << " vazias)\n";
    std::cout << "  AFD (subconjuntos): " << det.quantidadeDeEstados() << " estados\n";
    for (std::size_t i = 0; i < det.conjuntos.size(); ++i) {
        std::cout << "    q" << i << " = " << formatarConjunto(det.conjuntos[i]) << '\n';
    }
    std::cout << "  minimizacao: " << min.estadosAntes << " estados antes (erro incluido), " << min.estadosAlcancaveis
              << " alcancaveis, " << min.estadosDepois << " depois (" << min.rodadas << " rodadas)\n";
    std::cout << min.afd.formatarTabela();

    std::string linha;
    bool primeira = true;
    while (std::getline(std::cin, linha)) {
        if (primeira) {
            std::cout << "\nCadeias:\n";
            primeira = false;
        }
        std::istringstream palavras(linha);
        std::vector<Simbolo> cadeia;
        Simbolo s;
        while (palavras >> s) {
            cadeia.push_back(s);
        }
        std::cout << "  \"" << linha << "\": AFN " << (afn.aceita(cadeia) ? "aceita" : "recusa") << ", AFD "
                  << (det.afd.aceita(cadeia) ? "aceita" : "recusa") << ", minimo "
                  << (min.afd.aceita(cadeia) ? "aceita" : "recusa") << '\n';
    }
    return 0;
}
