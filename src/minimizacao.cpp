#include "minimizacao.h"

#include <map>
#include <queue>

namespace komboscript {

ResultadoDaMinimizacao minimizar(const AfdCombo& afd) {
    ResultadoDaMinimizacao resultado;

    const std::size_t total = afd.quantidadeDeEstados();
    const std::vector<Simbolo>& alfabeto = afd.alfabeto();
    const std::size_t colunas = alfabeto.size();
    resultado.estadosAntes = total;

    // Passo 1 — a tabela completa, extraída uma vez (a consulta por símbolo
    // passa por um mapa de strings; nas rodadas seguintes só se usa índice).
    std::vector<std::vector<Estado>> delta(total, std::vector<Estado>(colunas));
    for (Estado estado = 0; estado < total; ++estado) {
        for (std::size_t coluna = 0; coluna < colunas; ++coluna) {
            delta[estado][coluna] = afd.transicao(estado, alfabeto[coluna]);
        }
    }

    // Passo 2 — inacessíveis: busca em largura a partir do inicial.
    std::vector<char> acessivel(total, 0);
    std::vector<Estado> ordemBfs;
    std::queue<Estado> fila;
    acessivel[afd.estadoInicial()] = 1;
    fila.push(afd.estadoInicial());
    while (!fila.empty()) {
        const Estado atual = fila.front();
        fila.pop();
        ordemBfs.push_back(atual);
        for (std::size_t coluna = 0; coluna < colunas; ++coluna) {
            const Estado destino = delta[atual][coluna];
            if (!acessivel[destino]) {
                acessivel[destino] = 1;
                fila.push(destino);
            }
        }
    }
    resultado.estadosAlcancaveis = ordemBfs.size();

    // Passo 3 — partição inicial: aceitação x não aceitação, só entre os acessíveis.
    constexpr std::size_t kSem = ResultadoDaMinimizacao::kInacessivel;
    std::vector<std::size_t> classe(total, kSem);
    std::size_t quantidadeDeClasses = 0;
    {
        std::size_t idAceita = kSem;
        std::size_t idRecusa = kSem;
        for (const Estado estado : ordemBfs) {
            std::size_t& id = afd.ehDeAceitacao(estado) ? idAceita : idRecusa;
            if (id == kSem) {
                id = quantidadeDeClasses++;
            }
            classe[estado] = id;
        }
    }

    // Passo 4 — refinamento. A assinatura de um estado é (sua classe, classe
    // de cada destino). Estados com a mesma assinatura continuam juntos. A
    // partição só se parte, nunca se junta: se o número de classes não mudou,
    // a partição é a mesma e este é o ponto fixo.
    while (true) {
        ++resultado.rodadas;
        std::map<std::vector<std::size_t>, std::size_t> ids;
        std::vector<std::size_t> nova(total, kSem);
        for (const Estado estado : ordemBfs) {
            std::vector<std::size_t> assinatura;
            assinatura.reserve(colunas + 1);
            assinatura.push_back(classe[estado]);
            for (std::size_t coluna = 0; coluna < colunas; ++coluna) {
                assinatura.push_back(classe[delta[estado][coluna]]);
            }
            const auto [it, inserido] = ids.emplace(std::move(assinatura), ids.size());
            nova[estado] = it->second;
            (void)inserido;
        }
        const bool pontoFixo = ids.size() == quantidadeDeClasses;
        classe = std::move(nova);
        quantidadeDeClasses = ids.size();
        if (pontoFixo) {
            break;
        }
    }
    resultado.estadosDepois = quantidadeDeClasses;

    // Passo 5 — reconstrução. A classe do erro original (se alcançável) vira
    // o erro do novo AfdCombo; as demais são numeradas na ordem em que a
    // busca em largura as encontra, o que põe o inicial em 0.
    const std::size_t classeDoPoco = acessivel[afd.estadoDeErro()] ? classe[afd.estadoDeErro()] : kSem;
    const std::size_t uteis = quantidadeDeClasses - (classeDoPoco == kSem ? 0 : 1);

    std::vector<std::size_t> novoIndice(quantidadeDeClasses, kSem);
    std::vector<Estado> representante(quantidadeDeClasses, 0);
    std::size_t proximo = 0;
    for (const Estado estado : ordemBfs) {
        const std::size_t c = classe[estado];
        if (c == classeDoPoco) {
            continue;
        }
        if (novoIndice[c] == kSem) {
            novoIndice[c] = proximo++;
            representante[c] = estado;
        }
    }

    AfdCombo minimo(alfabeto, uteis, novoIndice[classe[afd.estadoInicial()]] == kSem
                                         ? uteis  // o próprio inicial é o poço: linguagem vazia
                                         : novoIndice[classe[afd.estadoInicial()]]);
    for (std::size_t c = 0; c < quantidadeDeClasses; ++c) {
        if (c == classeDoPoco) {
            continue;
        }
        const Estado rep = representante[c];
        if (afd.ehDeAceitacao(rep)) {
            minimo.marcarAceitacao(novoIndice[c]);
        }
        for (std::size_t coluna = 0; coluna < colunas; ++coluna) {
            const std::size_t destino = classe[delta[rep][coluna]];
            if (destino != classeDoPoco) {
                minimo.definirTransicao(novoIndice[c], alfabeto[coluna], novoIndice[destino]);
            }
        }
    }
    resultado.afd = std::move(minimo);

    resultado.classeDe.assign(total, ResultadoDaMinimizacao::kInacessivel);
    for (const Estado estado : ordemBfs) {
        resultado.classeDe[estado] =
            classe[estado] == classeDoPoco ? uteis : novoIndice[classe[estado]];
    }
    return resultado;
}

}  // namespace komboscript
