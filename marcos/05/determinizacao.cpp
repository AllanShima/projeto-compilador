#include "determinizacao.h"

#include <algorithm>
#include <map>
#include <queue>

namespace komboscript {

namespace {

constexpr std::size_t kSemDestino = static_cast<std::size_t>(-1);

std::vector<Estado> fechoOrdenado(const AfnCombo& afn, const std::vector<Estado>& conjunto) {
    std::vector<Estado> fecho = afn.fechoVazio(conjunto);
    std::sort(fecho.begin(), fecho.end());
    return fecho;
}

}  // namespace

std::size_t ResultadoDaDeterminizacao::quantidadeDeEstados() const {
    return conjuntos.size() + (vazioAlcancavel ? 1 : 0);
}

std::string formatarConjunto(const std::vector<Estado>& conjunto) {
    std::string texto = "{";
    for (std::size_t i = 0; i < conjunto.size(); ++i) {
        if (i > 0) {
            texto += ", ";
        }
        texto += std::to_string(conjunto[i]);
    }
    return texto + "}";
}

ResultadoDaDeterminizacao determinizar(const AfnCombo& afn, const std::vector<Simbolo>& alfabeto) {
    ResultadoDaDeterminizacao resultado;

    // Catálogo: conjunto de estados do AFN -> índice do estado do AFD. Os
    // conjuntos são sempre ordenados e sem repetição, e é isso que faz dois
    // caminhos que chegam ao mesmo conjunto caírem no mesmo estado.
    std::map<std::vector<Estado>, Estado> catalogo;
    std::queue<Estado> pendentes;

    auto catalogar = [&](std::vector<Estado> conjunto) {
        const auto achado = catalogo.find(conjunto);
        if (achado != catalogo.end()) {
            return achado->second;
        }
        const Estado novo = resultado.conjuntos.size();
        catalogo.emplace(conjunto, novo);
        resultado.conjuntos.push_back(std::move(conjunto));
        pendentes.push(novo);
        return novo;
    };

    catalogar(fechoOrdenado(afn, {afn.estadoInicial()}));

    // destinos[estado][coluna]: kSemDestino é o conjunto vazio (erro).
    std::vector<std::vector<std::size_t>> destinos;

    while (!pendentes.empty()) {
        const Estado atual = pendentes.front();
        pendentes.pop();
        if (destinos.size() <= atual) {
            destinos.resize(atual + 1);
        }
        destinos[atual].assign(alfabeto.size(), kSemDestino);

        for (std::size_t coluna = 0; coluna < alfabeto.size(); ++coluna) {
            // Copia: `catalogar` pode realocar `conjuntos`.
            const std::vector<Estado> origem = resultado.conjuntos[atual];
            const std::vector<Estado> alcancados = afn.mover(origem, alfabeto[coluna]);
            const std::vector<Estado> proximo = fechoOrdenado(afn, alcancados);
            if (proximo.empty()) {
                resultado.vazioAlcancavel = true;
                continue;
            }
            destinos[atual][coluna] = catalogar(proximo);
        }
    }

    // Monta o AfdCombo: n estados úteis, o erro vem logo depois.
    const std::size_t quantidade = resultado.conjuntos.size();
    AfdCombo afd(alfabeto, quantidade, 0);
    for (Estado estado = 0; estado < quantidade; ++estado) {
        if (quantidade <= 64) {
            afd.nomearEstado(estado, formatarConjunto(resultado.conjuntos[estado]));
        }
        const std::vector<Estado>& conjunto = resultado.conjuntos[estado];
        if (std::binary_search(conjunto.begin(), conjunto.end(), afn.estadoDeAceitacao())) {
            afd.marcarAceitacao(estado);
        }
        for (std::size_t coluna = 0; coluna < alfabeto.size(); ++coluna) {
            if (destinos[estado][coluna] != kSemDestino) {
                afd.definirTransicao(estado, alfabeto[coluna], destinos[estado][coluna]);
            }
        }
    }
    resultado.afd = std::move(afd);
    return resultado;
}

ResultadoDaDeterminizacao determinizar(const AfnCombo& afn) {
    return determinizar(afn, afn.simbolosUsados());
}

}  // namespace komboscript
