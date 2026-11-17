cmake_minimum_required(VERSION 3.14)
project(komboscript VERSION 0.5.0 LANGUAGES CXX)

# Padrao declarado uma vez, aqui, e nao repetido por compilador.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Aviso e erro. Num programa que manipula indices de tabela o tempo inteiro, um
# aviso de conversao implicita ignorado e um defeito adiado, nao evitado.
function(komboscript_avisos alvo)
    if(MSVC)
        target_compile_options(${alvo} PRIVATE /W4 /WX /permissive- /utf-8 /EHsc)
    else()
        target_compile_options(${alvo} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endfunction()

# --- Biblioteca ----------------------------------------------------------------
#   expressao     leitura da expressao -> arvore reduzida ao nucleo
#   afd           AFD (tabela de transicao densa)
#   afn           AFN: fecho vazio, movimento, simulacao
#   thompson      arvore -> AFN
#   determinizacao  AFN -> AFD (subconjuntos)
#   minimizacao   AFD -> AFD minimo (refinamento de particoes)
add_library(komboscript STATIC
    src/expressao.cpp
    src/afd.cpp
    src/afn.cpp
    src/thompson.cpp
    src/determinizacao.cpp
    src/minimizacao.cpp
)
target_include_directories(komboscript PUBLIC include)
komboscript_avisos(komboscript)

# --- Programa ------------------------------------------------------------------
# Do padrao ao AFD minimo: ./build/komboscript "[a | b]* -> c" a b c
add_executable(kombo main.cpp)
target_link_libraries(kombo PRIVATE komboscript)
komboscript_avisos(kombo)

# --- Demonstracoes dos marcos antigos ------------------------------------------
add_executable(demo_expressao demos/demo_expressao.cpp)
target_link_libraries(demo_expressao PRIVATE komboscript)
komboscript_avisos(demo_expressao)

add_executable(demo_afd demos/demo_afd.cpp)
target_link_libraries(demo_afd PRIVATE komboscript)
komboscript_avisos(demo_afd)

# --- Testes (ctest) ------------------------------------------------------------
# Um programa por parte. O diretorio de trabalho e a raiz do repositorio e o
# caminho dos casos vai como argumento: sem isso os testes reprovariam por nao
# achar o arquivo — falha por motivo que nada tem a ver com o que medem.
include(CTest)
enable_testing()

add_library(komboscript_testes STATIC tests/bateria.cpp tests/comparacao.cpp)
target_include_directories(komboscript_testes PUBLIC tests)
target_link_libraries(komboscript_testes PUBLIC komboscript)
komboscript_avisos(komboscript_testes)

foreach(parte IN ITEMS thompson afn determinizacao minimizacao)
    add_executable(test_${parte} tests/test_${parte}.cpp)
    target_link_libraries(test_${parte} PRIVATE komboscript_testes)
    komboscript_avisos(test_${parte})
    add_test(NAME ${parte}
             COMMAND test_${parte} "${CMAKE_CURRENT_SOURCE_DIR}/tests/casos.txt"
             WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endforeach()
