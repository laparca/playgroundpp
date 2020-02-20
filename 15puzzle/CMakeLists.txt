add_compile_options(-Wall -Wextra -pedantic -Werror -g3)

add_executable(15puzzle_normal 15puzzle.cpp)
add_executable(15puzzle_visited 15puzzle.cpp)
add_executable(15puzzle_sorted_visited 15puzzle.cpp)
add_executable(15puzzle_clean 15puzzle.cpp)

target_compile_features(15puzzle_normal PUBLIC cxx_std_17)
target_compile_features(15puzzle_visited PUBLIC cxx_std_17)
target_compile_features(15puzzle_sorted_visited PUBLIC cxx_std_17)
target_compile_features(15puzzle_clean PUBLIC cxx_std_17)
#target_compile_options(15puzzle2 PUBLIC -pg)

target_compile_options(15puzzle_visited PUBLIC -DUSE_VISITED)
target_compile_options(15puzzle_sorted_visited PUBLIC -DUSE_VISITED -DSORTED_VISITED)
target_compile_options(15puzzle_clean PUBLIC -DCLEAN_MEMORY)

#SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -pg")

