add_compile_options(-Wall -Wextra -pedantic -Werror -g3)

add_executable(go_chanel queue001.cpp)
target_compile_features(go_chanel PUBLIC cxx_std_20)

