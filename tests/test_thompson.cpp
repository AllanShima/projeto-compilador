// Parte 1 — construção de Thompson: árvore -> AFN.
// Confere as contagens (estados / transições / vazias) da tabela "Testes de
// Thompson" do guia, na notação do KomboScript.

#include <iostream>
#include <string>
#include <vector>

#include "ajuda.h"
#include "thompson.h"
#include "verificar.h"

using namespace komboscript;
using namespace teste;

namespace {

struct ContagemEsperada {
    std::string expressao;
    std::size_t estados;
    std::size_t transicoes;
    std::size_t vazias;
};

}  // namespace

int main() {
    std::cout << "== Thompson: contagens (estados / transicoes / vazias) ==\n";
    // `a.c` do guia usa coringa, que a linguagem não tem: fica de fora.
    const std::vector<ContagemEsperada> tabela = {
        {"a", 2, 1, 0},
        {"a -> b", 4, 3, 1},
        {"a | b", 6, 6, 4},
        {"a*", 4, 5, 4},
        {"[a | b]* -> c", 10, 12, 9},
        {"[a -> b -> c]*", 8, 9, 6},
        // Reduções do açúcar, que já chegam ao Thompson reduzidas ao núcleo.
        // a+ = concat(a, fecho(a)): 2 + 4 estados; 1 + 5 transições + a vazia da concatenação.
        {"a+", 6, 7, 5},
        // a? = alt(a, vazio): o ramo vazio já é uma transição vazia, além das 4 da alternância.
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

    std::cout << "== Thompson recebe a arvore, nao o texto ==\n";
    // A mesma árvore, lida uma vez, dá o mesmo AFN que a expressão passada
    // pelo atalho do teste.
    const Resultado leitura = analisarExpressao("[a | b]* -> c");
    verificar(leitura.ok, "leitura de (a|b)*c");
    const AfnCombo daArvore = construirThompson(leitura.arvore);
    verificarIgual(daArvore.quantidadeDeEstados(), std::size_t{10}, "(a|b)*c pela arvore: 10 estados");
    verificarIgual(daArvore.quantidadeDeTransicoes(), std::size_t{12}, "(a|b)*c pela arvore: 12 transicoes");
    verificarIgual(daArvore.quantidadeDeTransicoesVazias(), std::size_t{9}, "(a|b)*c pela arvore: 9 vazias");

    // Árvore vazia: um único estado, inicial e de aceitação.
    const AfnCombo vazio = construirThompson(Arvore{});
    verificarIgual(vazio.quantidadeDeEstados(), std::size_t{1}, "arvore vazia: 1 estado");
    verificar(vazio.aceita({}), "arvore vazia aceita a cadeia vazia");
    verificar(!vazio.aceita({"a"}), "arvore vazia recusa qualquer simbolo");

    // O fecho aninhado (ciclo de transições vazias) não pode travar a construção.
    const AfnCombo duplo = afnDe("a**");
    verificar(duplo.aceita({}), "a** aceita a cadeia vazia");
    verificar(duplo.aceita({"a", "a", "a"}), "a** aceita aaa");
    verificar(!duplo.aceita({"b"}), "a** recusa b");

    return resumo();
}
