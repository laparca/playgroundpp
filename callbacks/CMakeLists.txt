project(callbacks)
add_compile_options(-Wall -Wextra -pedantic -Werror -g3)
add_executable(callbacks callbacks.cc)
target_compile_features(callbacks PUBLIC cxx_std_11)
