project(metaprogramming_15puzzle)

add_compile_options(-ftemplate-depth=999999 -Wall)

add_executable(15puzzle 15puzzle.cpp)
target_compile_features(15puzzle PUBLIC cxx_std_17)

add_executable(15puzzle_a_star 15puzzle_a_star.cpp)
target_compile_features(15puzzle_a_star PUBLIC cxx_std_17)

