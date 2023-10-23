project(coro)

add_executable(coro_test main.cc)
target_compile_options(coro_test PUBLIC -Wall -Wextra -pedantic -Werror -g3 )
#target_link_libraries(go_chanel_test gtest gtest_main gmock)
target_compile_features(coro_test PRIVATE cxx_std_20)
#target_include_directories(go_chanel_test PUBLIC . include)
#enable_testing()
#add_test(NAME go_chanel_test COMMAND go_chanel_test)