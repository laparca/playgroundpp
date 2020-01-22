#include <iostream>
#include <playgroundcpp/group_comparator.hh>

int getValue() {
    return rand() % 5;
}
int main() {
    using namespace playgroundcpp::group_comparator;

    for (int i = 0; i < 100; i++) {
        auto v = getValue();
        std::cout << i << "[v=" << v << "]: ";
        if (for_any_of(1, 2, 3, 4) > v)
            std::cout << "yes";
        else
            std::cout << "no";
        std::cout << std::endl;
    }

    if (for_any_of(1, 2, 3, 4) < 3)
        std::cout << "yes";
    else
        std::cout << "no";
    std::cout << std::endl;

    if (for_all_of(1, 2, 3, 4) < 3)
        std::cout << "yes";
    else
        std::cout << "no";
    std::cout << std::endl;

    if (for_all_of(1, 2, 3, 4) < 5)
        std::cout << "yes";
    else
        std::cout << "no";
    std::cout << std::endl;

    for (int i = 0; i < 100; i++) {
        auto v = getValue();
        std::cout << i << "[v=" << v << "]: ";
        if (for_any_of(1, 2, 3, 4) == v)
            std::cout << "yes";
        else
            std::cout << "no";
        std::cout << std::endl;
    }

    std::cout <<(getValue() == for_any_of(1, 4) ? "yes" : "no") << std::endl;
    
#if __cplusplus >= 201703L
    if (all_of{1, 2, 3, 4} < 5)
        std::cout << "yes";
    else
        std::cout << "no";
    std::cout << std::endl;

    if (5 > all_of{1, 2, 3, 4})
        std::cout << "yes";
    else
        std::cout << "no";
    std::cout << std::endl;

#endif
    return 0;
}