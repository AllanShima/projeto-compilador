#ifndef KOMBOSCRIPT_KOMBO_AFN_H
#define KOMBOSCRIPT_KOMBO_AFN_H

#include <cstddef>
#include <string>
#include <vector>

#include "afd.h" // Inclui o AFD do seu projeto que define Estado e Simbolo

namespace komboscript {

// Rótulo da transição vazia para sequências de combos, representado por string vazia.
inline const Simbolo kEpsilon = "";

// Rótulo de coringa para combos (caso queiram aceitar qualquer input válido naquele frame).
inline const Simbolo kQualquer = "*";

struct TransicaoAfn {
    Simbolo simbolo = kEpsilon;
    Estado destino = 0;
};

// Um bloco de máquina com uma entrada e uma saída — o invariante da construção de Thompson.
struct Bloco {
    Estado entrada = 0;
    Estado saida = 0;
};

// Um passo do traço da simulação do AFN de combos: estados ativos antes de iniciar
// ou logo após ler um símbolo/input do jogador.
struct PassoDaSimulacaoAfn {
    Simbolo simboloLido;
    std::vector<Estado> ativos;
};

struct SimulacaoAfn {
    bool aceitou = false;
    std::vector<PassoDaSimulacaoAfn> passos;
    std::size_t maiorConjuntoAtivo = 0;
};

class AfnCombo {
public:
    AfnCombo(std::vector<Simbolo> alfabeto);
    AfnCombo();

    Estado novoEstado();
    void adicionarTransicao(Estado origem, const Simbolo& simbolo, Estado destino);

    void definirInicial(Estado estado);
    void definirAceitacao(Estado estado);

    std::size_t quantidadeDeEstados() const;
    std::size_t quantidadeDeTransicoes() const;
    std::size_t quantidadeDeTransicoesVazias() const;

    // O fecho vazio de um conjunto: estados alcançáveis sem consumir inputs.
    std::vector<Estado> fechoVazio(const std::vector<Estado>& conjunto) const;

    bool aceita(const std::vector<Simbolo>& sequencia) const;
    SimulacaoAfn simular(const std::vector<Simbolo>& sequencia) const;

    std::string formatarSimulacao(const std::vector<Simbolo>& sequencia, const SimulacaoAfn& simulacao) const;

    // Opcional: converte este AFN resultante de Thompson em um AfdCombo definitivo
    // utilizando o Algoritmo de Construção por Subconjuntos.
    // AfdCombo paraAfd() const;

private:
    std::vector<Simbolo> alfabeto_;
    std::vector<std::vector<TransicaoAfn>> transicoes_;
    Estado inicial_ = 0;
    Estado aceitacao_ = 0;
};

}  // namespace komboscript

#endif  // KOMBOSCRIPT_KOMBO_AFN_H