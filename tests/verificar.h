// verificar.h — o mínimo para escrever testes sem biblioteca externa.
//
// Cada teste é um programa com `main`; `ctest` roda um por arquivo. As
// verificações não param na primeira falha: o programa conta e, ao final,
// `resumo()` devolve o código de saída (0 se tudo passou).

#ifndef KOMBOSCRIPT_TESTES_VERIFICAR_H
#define KOMBOSCRIPT_TESTES_VERIFICAR_H

#include <cstddef>
#include <iostream>
#include <string>

namespace teste {

inline std::size_t verificacoes = 0;
inline std::size_t falhas = 0;

inline void verificar(const bool condicao, const std::string& descricao) {
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

// Imprime o resumo e devolve o código de saída do programa.
inline int resumo() {
    std::cout << "\n" << (verificacoes - falhas) << "/" << verificacoes << " verificacoes passaram.\n";
    return falhas == 0 ? 0 : 1;
}

}  // namespace teste

#endif  // KOMBOSCRIPT_TESTES_VERIFICAR_H
