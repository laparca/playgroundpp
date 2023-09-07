project(go_chanel)

add_executable(go_chanel go_chanel.cpp)

target_compile_options(go_chanel PRIVATE -Wall -Wextra -pedantic -Werror -g3)
target_compile_features(go_chanel PUBLIC cxx_std_20)
target_include_directories(go_chanel PUBLIC . include)

add_executable(go_chanel_test chanel_test.cpp)
target_compile_options(go_chanel_test PUBLIC -Wall -Wextra -pedantic -Werror -g3)
target_link_libraries(go_chanel_test gtest gtest_main gmock)
target_compile_features(go_chanel_test PRIVATE cxx_std_20)
target_include_directories(go_chanel_test PUBLIC . include)
enable_testing()
add_test(NAME go_chanel_test COMMAND go_chanel_test)