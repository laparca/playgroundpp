add_compile_options(-Wall -Wextra -pedantic -Werror -g3)

add_executable(15puzzle2 15puzzle.cpp)
target_compile_features(15puzzle2 PUBLIC cxx_std_17)
target_compile_options(15puzzle2 PUBLIC -pg)

SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -pg")

