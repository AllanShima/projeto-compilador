#include "afn.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <utility>

namespace komboscript {

AfnCombo::AfnCombo(std::vector<Simbolo> alfabeto)
    : alfabeto_(std::move(alfabeto)), inicial_(0), aceitacao_(0) {}

AfnCombo::AfnCombo() : inicial_(0), aceitacao_(0) {}

Estado AfnCombo::novoEstado() {
    transicoes_.emplace_back();
    return transicoes_.size() - 1;
}

void AfnCombo::adicionarTransicao(const Estado origem, const Simbolo& simbolo, const Estado destino) {
    if (origem >= transicoes_.size()) {
        transicoes_.resize(origem + 1);
    }
    transicoes_[origem].push_back(Transicao{simbolo, destino});
}

void AfnCombo::definirInicial(const Estado estado) { inicial_ = estado; }
void AfnCombo::definirAceitacao(const Estado estado) { aceitacao_ = estado; }

std::size_t AfnCombo::quantidadeDeEstados() const { return transicoes_.size(); }

std::size_t AfnCombo::quantidadeDeTransicoes() const {
    std::size_t total = 0;
    for (const std::vector<Transicao>& saidas : transicoes_) {
        total += saidas.size();
    }
    return total;
}

std::size_t AfnCombo::quantidadeDeTransicoesVazias() const {
    std::size_t total = 0;
    for (const std::vector<Transicao>& saidas : transicoes_) {
        for (const Transicao& transicao : saidas) {
            if (transicao.simbolo == kEpsilon) {
                ++total;
            }
        }
    }
    return total;
}

// Busca em profundidade. A marcação de visitados evita laço infinito em
// ciclos de transições vazias (ex.: Jab**).
std::vector<Estado> AfnCombo::fechoVazio(const std::vector<Estado>& conjunto) const {
    std::vector<char> visitado(transicoes_.size(), 0);
    std::vector<Estado> pilha;
    std::vector<Estado> resultado;

    for (const Estado estado : conjunto) {
        if (estado < transicoes_.size() && !visitado[estado]) {
            visitado[estado] = 1;
            pilha.push_back(estado);
            resultado.push_back(estado);
        }
    }

    while (!pilha.empty()) {
        const Estado atual = pilha.back();
        pilha.pop_back();
        for (const Transicao& transicao : transicoes_[atual]) {
            if (transicao.simbolo != kEpsilon) {
                continue;
            }
            if (transicao.destino < transicoes_.size() && !visitado[transicao.destino]) {
                visitado[transicao.destino] = 1;
                pilha.push_back(transicao.destino);
                resultado.push_back(transicao.destino);
            }
        }
    }
    return resultado;
}

std::vector<Estado> AfnCombo::mover(const std::vector<Estado>& conjunto, const Simbolo& simbolo) const {
    std::vector<Estado> alcancados;
    for (const Estado estado : conjunto) {
        if (estado >= transicoes_.size()) {
            continue;
        }
        for (const Transicao& transicao : transicoes_[estado]) {
            if (transicao.simbolo == kEpsilon) {
                continue;
            }
            if (transicao.simbolo == simbolo || (transicao.simbolo == kQualquer && simbolo != kEpsilon)) {
                alcancados.push_back(transicao.destino);
            }
        }
    }
    std::sort(alcancados.begin(), alcancados.end());
    alcancados.erase(std::unique(alcancados.begin(), alcancados.end()), alcancados.end());
    return alcancados;
}

Estado AfnCombo::estadoInicial() const { return inicial_; }
Estado AfnCombo::estadoDeAceitacao() const { return aceitacao_; }

std::vector<Simbolo> AfnCombo::simbolosUsados() const {
    std::vector<Simbolo> simbolos;
    for (const std::vector<Transicao>& saidas : transicoes_) {
        for (const Transicao& transicao : saidas) {
            if (transicao.simbolo != kEpsilon && transicao.simbolo != kQualquer) {
                simbolos.push_back(transicao.simbolo);
            }
        }
    }
    std::sort(simbolos.begin(), simbolos.end());
    simbolos.erase(std::unique(simbolos.begin(), simbolos.end()), simbolos.end());
    return simbolos;
}

// Tarefa 2: mantém o conjunto de estados ativos. Aceita se o estado de
// aceitação estiver entre eles no fim.
SimulacaoAfn AfnCombo::simular(const std::vector<Simbolo>& sequencia) const {
    SimulacaoAfn simulacao;

    // O conjunto inicial já inclui o fecho vazio.
    std::vector<Estado> ativos = fechoVazio({inicial_});
    simulacao.passos.push_back(PassoDaSimulacaoAfn{kEpsilon, ativos});
    simulacao.maiorConjuntoAtivo = ativos.size();

    std::vector<char> presente(transicoes_.size(), 0);

    for (const Simbolo& simbolo : sequencia) {
        std::vector<Estado> proximos;
        for (const Estado estado : ativos) {
            for (const Transicao& transicao : transicoes_[estado]) {
                // Transição vazia nunca consome input.
                if (transicao.simbolo == kEpsilon) {
                    continue;
                }
                const bool casa = transicao.simbolo == simbolo ||
                                  (transicao.simbolo == kQualquer && simbolo != kEpsilon);
                if (!casa) {
                    continue;
                }
                if (transicao.destino < presente.size() && !presente[transicao.destino]) {
                    presente[transicao.destino] = 1;
                    proximos.push_back(transicao.destino);
                }
            }
        }
        for (const Estado estado : proximos) {
            presente[estado] = 0;
        }

        ativos = fechoVazio(proximos);
        simulacao.passos.push_back(PassoDaSimulacaoAfn{simbolo, ativos});
        if (ativos.size() > simulacao.maiorConjuntoAtivo) {
            simulacao.maiorConjuntoAtivo = ativos.size();
        }
        if (ativos.empty()) {
            break;
        }
    }

    for (const Estado estado : ativos) {
        if (estado == aceitacao_) {
            simulacao.aceitou = true;
            break;
        }
    }
    return simulacao;
}

bool AfnCombo::aceita(const std::vector<Simbolo>& sequencia) const {
    return simular(sequencia).aceitou;
}

std::string AfnCombo::formatarSimulacao(const std::vector<Simbolo>& sequencia,
                                        const SimulacaoAfn& simulacao) const {
    std::string texto = "  combo sobre \"";
    for (std::size_t i = 0; i < sequencia.size(); ++i) {
        if (i > 0) {
            texto += ' ';
        }
        texto += sequencia[i];
    }
    texto += "\"\n";
    for (const PassoDaSimulacaoAfn& passo : simulacao.passos) {
        texto += "    ";
        if (passo.simboloLido == kEpsilon) {
            texto += "inicio ";
        } else {
            texto += "apos '" + passo.simboloLido + "' ";
        }
        texto += "-> { ";
        for (std::size_t i = 0; i < passo.ativos.size(); ++i) {
            if (i > 0) {
                texto += ", ";
            }
            texto += std::to_string(passo.ativos[i]);
        }
        texto += " }\n";
    }
    texto += std::string("  resultado: ") + (simulacao.aceitou ? "ACEITA" : "RECUSA");
    texto += " | maior conjunto ativo: " + std::to_string(simulacao.maiorConjuntoAtivo) + "\n";
    return texto;
}

}  // namespace komboscript
