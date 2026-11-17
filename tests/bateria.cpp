#include "bateria.h"

#include <fstream>
#include <sstream>

#include "thompson.h"

namespace komboscript {

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
