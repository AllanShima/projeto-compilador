// AFN do KomboScript, construção de Thompson e bateria de testes.

#ifndef KOMBOSCRIPT_KOMBO_AFN_H
#define KOMBOSCRIPT_KOMBO_AFN_H

#include <cstddef>
#include <string>
#include <vector>

#include "afd.h"
#include "expressao.h"  // Arvore

namespace komboscript {

// Transição vazia (não consome input).
inline const Simbolo kEpsilon = "";

// Reservado para coringa. A linguagem não tem coringa; a construção nunca o usa.
inline const Simbolo kQualquer = "*";

struct TransicaoAfn {
    Simbolo simbolo = kEpsilon;
    Estado destino = 0;
};

// Bloco com uma entrada e uma saída.
struct Bloco {
    Estado entrada = 0;
    Estado saida = 0;
};

// Estados ativos antes do primeiro input (simboloLido == kEpsilon) ou após um input.
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

    // Estados alcançáveis sem consumir input.
    std::vector<Estado> fechoVazio(const std::vector<Estado>& conjunto) const;

    bool aceita(const std::vector<Simbolo>& sequencia) const;
    SimulacaoAfn simular(const std::vector<Simbolo>& sequencia) const;

    std::string formatarSimulacao(const std::vector<Simbolo>& sequencia,
                                  const SimulacaoAfn& simulacao) const;

private:
    std::vector<Simbolo> alfabeto_;
    std::vector<std::vector<TransicaoAfn>> transicoes_;
    Estado inicial_ = 0;
    Estado aceitacao_ = 0;
};

// Tarefa 1: árvore -> AFN.
AfnCombo construirThompson(const Arvore& arvore);

// Tarefa 3: bateria de casos.
struct CasoDeTeste {
    std::string expressao;
    std::vector<Simbolo> sequencia;
    bool esperado = false;
    std::string origem;         // arquivo:linha
    std::string erroDeFormato;  // preenchido se a linha for inválida
};

struct ResultadoDaBateria {
    std::size_t total = 0;
    std::size_t passaram = 0;
    std::vector<std::string> falhas;
};

// Formato da linha: expressao <TAB> inputs separados por espaço <TAB> aceita|recusa
// Devolve lista vazia se o arquivo não abre.
std::vector<CasoDeTeste> lerCasos(const std::string& caminho);

// Roda todos os casos, sem parar na primeira falha.
ResultadoDaBateria rodarBateria(const std::vector<CasoDeTeste>& casos);

}  // namespace komboscript

#endif  // KOMBOSCRIPT_KOMBO_AFN_H
