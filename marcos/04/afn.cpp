#include "afn.h"

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
    transicoes_[origem].push_back(TransicaoAfn{simbolo, destino});
}

void AfnCombo::definirInicial(const Estado estado) { inicial_ = estado; }
void AfnCombo::definirAceitacao(const Estado estado) { aceitacao_ = estado; }

std::size_t AfnCombo::quantidadeDeEstados() const { return transicoes_.size(); }

std::size_t AfnCombo::quantidadeDeTransicoes() const {
    std::size_t total = 0;
    for (const std::vector<TransicaoAfn>& saidas : transicoes_) {
        total += saidas.size();
    }
    return total;
}

std::size_t AfnCombo::quantidadeDeTransicoesVazias() const {
    std::size_t total = 0;
    for (const std::vector<TransicaoAfn>& saidas : transicoes_) {
        for (const TransicaoAfn& transicao : saidas) {
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
        for (const TransicaoAfn& transicao : transicoes_[atual]) {
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
            for (const TransicaoAfn& transicao : transicoes_[estado]) {
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

// --- Tarefa 1: construção de Thompson ---

namespace {

// Um ramo por tipo de nó. Cada um devolve um bloco com uma entrada e uma saída.
Bloco construir(const Arvore& arvore, const std::size_t indice, AfnCombo& afn) {
    const No& no = arvore.nos[indice];
    switch (no.tipo) {
        case TipoDeNo::Identificador:
        case TipoDeNo::Vazio: {
            // Dois estados e uma transição (rótulo = nome, ou vazia).
            const Estado entrada = afn.novoEstado();
            const Estado saida = afn.novoEstado();
            const Simbolo rotulo = no.tipo == TipoDeNo::Identificador ? no.nome : kEpsilon;
            afn.adicionarTransicao(entrada, rotulo, saida);
            return Bloco{entrada, saida};
        }
        case TipoDeNo::Concatenacao: {
            const Bloco esquerdo = construir(arvore, no.esquerda, afn);
            const Bloco direito = construir(arvore, no.direita, afn);
            afn.adicionarTransicao(esquerdo.saida, kEpsilon, direito.entrada);
            return Bloco{esquerdo.entrada, direito.saida};
        }
        case TipoDeNo::Alternancia: {
            const Bloco esquerdo = construir(arvore, no.esquerda, afn);
            const Bloco direito = construir(arvore, no.direita, afn);
            const Estado entrada = afn.novoEstado();
            const Estado saida = afn.novoEstado();
            afn.adicionarTransicao(entrada, kEpsilon, esquerdo.entrada);
            afn.adicionarTransicao(entrada, kEpsilon, direito.entrada);
            afn.adicionarTransicao(esquerdo.saida, kEpsilon, saida);
            afn.adicionarTransicao(direito.saida, kEpsilon, saida);
            return Bloco{entrada, saida};
        }
        case TipoDeNo::Fecho: {
            const Bloco interno = construir(arvore, no.esquerda, afn);
            const Estado entrada = afn.novoEstado();
            const Estado saida = afn.novoEstado();
            afn.adicionarTransicao(entrada, kEpsilon, interno.entrada);        // uma vez ou mais
            afn.adicionarTransicao(entrada, kEpsilon, saida);                  // zero vezes
            afn.adicionarTransicao(interno.saida, kEpsilon, interno.entrada);  // repetir
            afn.adicionarTransicao(interno.saida, kEpsilon, saida);            // terminar
            return Bloco{entrada, saida};
        }
    }
    return Bloco{0, 0};  // inalcançável; evita aviso do compilador
}

}  // namespace

AfnCombo construirThompson(const Arvore& arvore) {
    AfnCombo afn;
    if (arvore.vazia()) {
        // Um único estado: aceita só a sequência vazia.
        const Estado unico = afn.novoEstado();
        afn.definirInicial(unico);
        afn.definirAceitacao(unico);
        return afn;
    }
    const Bloco raiz = construir(arvore, arvore.raiz, afn);
    afn.definirInicial(raiz.entrada);
    afn.definirAceitacao(raiz.saida);
    return afn;
}

// --- Tarefa 3: bateria de casos ---

namespace {

std::vector<Simbolo> separarInputs(const std::string& campo) {
    std::vector<Simbolo> inputs;
    std::istringstream palavras(campo);
    Simbolo palavra;
    while (palavras >> palavra) {
        inputs.push_back(palavra);
    }
    return inputs;
}

}  // namespace

std::vector<CasoDeTeste> lerCasos(const std::string& caminho) {
    std::vector<CasoDeTeste> casos;
    std::ifstream arquivo(caminho);
    if (!arquivo) {
        return casos;
    }

    std::string linha;
    std::size_t numeroDaLinha = 0;
    while (std::getline(arquivo, linha)) {
        ++numeroDaLinha;
        if (!linha.empty() && linha.back() == '\r') {
            linha.pop_back();
        }
        if (linha.empty() || linha[0] == '#') {
            continue;
        }

        CasoDeTeste caso;
        caso.origem = caminho + ":" + std::to_string(numeroDaLinha);

        const std::size_t primeiraTab = linha.find('\t');
        const std::size_t segundaTab =
            primeiraTab == std::string::npos ? std::string::npos : linha.find('\t', primeiraTab + 1);
        if (segundaTab == std::string::npos) {
            caso.erroDeFormato = "linha sem os tres campos separados por tabulacao";
            casos.push_back(caso);
            continue;
        }

        caso.expressao = linha.substr(0, primeiraTab);
        caso.sequencia = separarInputs(linha.substr(primeiraTab + 1, segundaTab - primeiraTab - 1));
        const std::string veredito = linha.substr(segundaTab + 1);
        if (veredito == "aceita") {
            caso.esperado = true;
        } else if (veredito == "recusa") {
            caso.esperado = false;
        } else {
            caso.erroDeFormato = "veredito \"" + veredito + "\" nao e aceita nem recusa";
        }
        casos.push_back(caso);
    }
    return casos;
}

ResultadoDaBateria rodarBateria(const std::vector<CasoDeTeste>& casos) {
    ResultadoDaBateria resultado;
    for (const CasoDeTeste& caso : casos) {
        ++resultado.total;
        if (!caso.erroDeFormato.empty()) {
            resultado.falhas.push_back(caso.origem + ": " + caso.erroDeFormato);
            continue;
        }
        const Resultado leitura = analisarExpressao(caso.expressao);
        if (!leitura.ok) {
            resultado.falhas.push_back(caso.origem + ": expressao recusada pela leitura — " +
                                       leitura.erro.mensagem + " (posicao " +
                                       std::to_string(leitura.erro.posicao) + ")");
            continue;
        }
        const AfnCombo afn = construirThompson(leitura.arvore);
        const bool obtido = afn.aceita(caso.sequencia);
        if (obtido == caso.esperado) {
            ++resultado.passaram;
            continue;
        }
        std::string sequencia;
        for (std::size_t i = 0; i < caso.sequencia.size(); ++i) {
            if (i > 0) {
                sequencia += ' ';
            }
            sequencia += caso.sequencia[i];
        }
        resultado.falhas.push_back(caso.origem + ": /" + caso.expressao + "/ sobre \"" + sequencia +
                                   "\" — esperado " + (caso.esperado ? "aceita" : "recusa") +
                                   ", obtido " + (obtido ? "aceita" : "recusa"));
    }
    return resultado;
}

}  // namespace komboscript
