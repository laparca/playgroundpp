#ifndef __TLIST_H__
#define __TLIST_H__

#include <type_traits>
#include "nothing.h"

/* List that stores types in compilation time. A empty list is Nothing */
template<class Type, class Next>
struct TList {
    using type = Type;
    using next = Next;
};

template<class Type, class List>
struct tlist_has_element : std::conditional_t<
        std::is_same_v<Type, typename List::type>,
        std::true_type,
        typename tlist_has_element<Type, typename List::next>::type
    > {};

template<class Type>
struct tlist_has_element<Type, Nothing> : std::false_type {};

template<class Type, class List>
inline constexpr bool tlist_has_element_v = tlist_has_element<Type, List>::value;

static_assert(tlist_has_element_v<Long<0>, Nothing> == false);
static_assert(tlist_has_element_v<Long<0>, TList<Long<0>, Nothing>> == true);
static_assert(tlist_has_element_v<Long<0>, TList<Long<0>, TList<Long<1>, Nothing>>> == true);
static_assert(tlist_has_element_v<Long<0>, TList<Long<1>, TList<Long<0>, Nothing>>> == true);
static_assert(tlist_has_element_v<Long<0>, TList<Long<1>, TList<Long<2>, Nothing>>> == false);
static_assert(tlist_has_element_v<move_t<Right, Long<0x1234'5678'9abc'de0f>>, Nothing> == false);
static_assert(tlist_has_element_v<move_t<Right, Long<0x1234'5678'9abc'de0f>>, TList<Long<0x1234'5678'9abc'def0>, Nothing>> == true);

template<class List>
struct tlist_size {
    static constexpr uint64_t value = 1 + tlist_size<typename List::next>::value;
};

template<>
struct tlist_size<Nothing> {
    static constexpr uint64_t value = 0;
};

template<class List>
inline constexpr uint64_t tlist_size_v = tlist_size<List>::value;

static_assert(tlist_size_v<Nothing> == 0);
static_assert(tlist_size_v<TList<Long<0>, Nothing>> == 1);
static_assert(tlist_size_v<TList<Long<1>, TList<Long<0>, Nothing>>> == 2);

template<class List>
struct tlist_head {
    using type = typename List::type;
};

template<class List>
using tlist_head_t = typename tlist_head<List>::type;

template<class List>
struct tlist_tail {
    using type = typename List::next;
};

template<class List>
using tlist_tail_t = typename tlist_tail<List>::type;

template<class List, int P>
struct tlist_get : tlist_get<tlist_tail_t<List>, P-1> {};

template<class List>
struct tlist_get<List, 0> : List {};

template<int P>
struct tlist_get<Nothing, P> : Nothing {};

template<>
struct tlist_get<Nothing, 0> : Nothing {};

template<class List, int P>
using tlist_get_t = typename tlist_get<List, P>::type;

template<class Type, class List>
struct tlist_add {
    using type = TList<Type, List>;
};

template<class Type, class List>
using tlist_add_t = typename tlist_add<Type, List>::type;

template<class Type, class List>
struct tlist_add_unique {
    using type = std::conditional_t<
        tlist_has_element<Type, List>::type::value,
        List,
        tlist_add_t<Type, List>
    >;
};

template<class List>
struct tlist_add_unique<Nothing, List> {
    using type = List;
};

template<class Type, class List>
using tlist_add_unique_t = typename tlist_add_unique<Type, List>::type;

static_assert(std::is_same_v<tlist_add_unique_t<Long<0>, Nothing>, TList<Long<0>, Nothing>>);
static_assert(std::is_same_v<tlist_add_unique_t<Long<1>, TList<Long<0>, Nothing>>, TList<Long<1>, TList<Long<0>, Nothing>>>);
static_assert(std::is_same_v<tlist_add_unique_t<Long<0>, TList<Long<0>, Nothing>>, TList<Long<0>, Nothing>>);

template<typename Type, class List, template<typename, typename> typename SortCmp>
struct tlist_sort_add;

template<typename Type, class List, template<typename, typename> typename SortCmp>
using tlist_sort_add_t = typename tlist_sort_add<Type, List, SortCmp>::type;


template<bool TypeGoFirst, class Type, class List, template<typename, typename> typename SortCmp>
struct tlist_sort_add_helper;

template<bool TypeGoFirst, class Type, class List, template<typename, typename> typename SortCmp>
using tlist_sort_add_helper_t = typename tlist_sort_add_helper<TypeGoFirst, Type, List, SortCmp>::type;

template<class Type, class List, template<typename, typename> typename SortCmp>
struct tlist_sort_add_helper<true, Type, List, SortCmp> : tlist_add<Type, List> {};

template<class Type, class List, template<typename, typename> typename SortCmp>
struct tlist_sort_add_helper<false, Type, List, SortCmp> : tlist_add<
    tlist_head_t<List>,
    tlist_sort_add_helper_t<
        SortCmp<
            Type,
            tlist_get_t<List, 1>
        >::value,
        Type,
        tlist_tail_t<List>,
        SortCmp
    >
> {};

template<typename Type, class List, template<typename, typename> typename SortCmp>
struct tlist_sort_add : tlist_sort_add_helper<SortCmp<Type, tlist_head_t<List>>::value, Type, List, SortCmp> {};

template<bool IsRepeated, class Type, class List, template<typename, typename> typename SortCmp>
struct tlist_sort_add_unique : tlist_sort_add<Type, List, SortCmp> {};

template<class Type, class List, template<typename, typename> typename SortCmp>
struct tlist_sort_add_unique<true, Type, List, SortCmp> {
   using type = List;
};

template<typename Type, class List, template<typename, typename> typename SortCmp>
using tlist_sort_add_unique_t = typename tlist_sort_add_unique<
    tlist_has_element_v<Type, List>,
    Type,
    List,
    SortCmp
>::type;

template<typename A, typename B>
struct great_than {
    static constexpr bool value = A::value > B::value;
};
template<typename A>
struct great_than<A, Nothing> {
    static constexpr bool value = true;
};

template<typename A, typename B>
struct less_than {
    static constexpr bool value = A::value < B::value;
};
template<typename A>
struct less_than<A, Nothing> {
    static constexpr bool value = true;
};

static_assert(
    std::is_same_v<
        tlist_sort_add_t<
            Long<0>,
            Nothing,
            great_than
        >,
        TList<Long<0>, Nothing>
    >
);
static_assert(
    std::is_same_v<
        tlist_sort_add_t<
            Long<0>,
            TList<Long<1>, Nothing>,
            great_than
        >,
        TList<Long<1>, TList<Long<0>, Nothing>>
    >
);
static_assert(
    std::is_same_v<
        tlist_sort_add_t<
            Long<0>,
            TList<Long<2>, TList<Long<1>, Nothing>>,
            great_than
        >,
        TList<Long<2>, TList<Long<1>, TList<Long<0>, Nothing>>>
    >
);
static_assert(
    std::is_same_v<
        tlist_sort_add_t<
            Long<0>,
            TList<Long<1>, TList<Long<2>, Nothing>>,
            less_than
        >,
        TList<Long<0>, TList<Long<1>, TList<Long<2>, Nothing>>>
    >
);

/**
 * Implements bubble sort. Yes, it's aweful, but easy
 */
template<class List, template<typename, typename> typename SortCmp>
struct tlist_sort_by;

template<class List, template<typename, typename> typename SortCmp>
using tlist_sort_by_t = typename tlist_sort_by<List, SortCmp>::type;

template<class SortedList, class UnsortedList, template<typename, typename> typename SortCmp>
struct tlist_sort_by_helper : tlist_sort_by_helper<tlist_sort_add_t<tlist_head_t<UnsortedList>, SortedList, SortCmp>, tlist_tail_t<UnsortedList>, SortCmp> {};

template<class SortedList, template<typename, typename> typename SortCmp>
struct tlist_sort_by_helper<SortedList, Nothing, SortCmp> {
    using type = SortedList;
};

template<class List, template<typename, typename> typename SortCmp>
struct tlist_sort_by : tlist_sort_by_helper<Nothing, List, SortCmp> {};

static_assert(
    std::is_same_v<
        tlist_sort_by_t<
            TList<Long<2>, TList<Long<0>, TList<Long<3>, TList<Long<1>, Nothing>>>>, 
            less_than
        >,
        TList<Long<0>, TList<Long<1>, TList<Long<2>, TList<Long<3>, Nothing>>>>
    >
);

template<class List, template<class> typename If>
struct tlist_remove_if;

template<class List, template<class> typename If>
using tlist_remove_if_t = typename tlist_remove_if<List, If>::type;

template<bool IsSameThanHead, class List, template<typename> typename If>
struct tlist_remove_if_helper : tlist_add<
    tlist_head_t<List>,
    tlist_remove_if_t<tlist_tail_t<List>, If>
> {};

template<class List, template<class> typename If>
struct tlist_remove_if_helper<true, List, If> : tlist_tail<List> {};

template<bool IsSameThanHead, template<class> typename If>
struct tlist_remove_if_helper<IsSameThanHead, Nothing, If> : Nothing {};


template<class List, template<class> typename If>
struct tlist_remove_if : tlist_remove_if_helper<If<tlist_head_t<List>>::value, List, If> {};

template<template<class>typename If>
struct tlist_remove_if<Nothing, If> : Nothing {};

template<class T, class List>
struct tlist_remove {
    template<typename Other>
    using If = std::is_same<T, Other>;
    using type = tlist_remove_if_t<List, If>;
};

template<class T, class List>
using tlist_remove_t = typename tlist_remove<T, List>::type;

static_assert(std::is_same_v<tlist_remove_t<Long<0>, Nothing>, Nothing>);
static_assert(std::is_same_v<tlist_remove_t<Long<0>, TList<Long<0>, Nothing>>, Nothing>);
static_assert(std::is_same_v<tlist_remove_t<Long<0>, TList<Long<0>, TList<Long<1>, Nothing>>>, TList<Long<1>, Nothing>>);
static_assert(std::is_same_v<tlist_remove_t<Long<0>, TList<Long<1>, TList<Long<0>, Nothing>>>, TList<Long<1>, Nothing>>);

static_assert(std::is_same_v<tlist_remove_t<Long<0>, TList<Long<2>, TList<Long<0>, TList<Long<1>, Nothing>>>>, TList<Long<2>, TList<Long<1>, Nothing>>>);
static_assert(std::is_same_v<tlist_remove_t<Long<0>, TList<Long<1>, TList<Long<2>, Nothing>>>, TList<Long<1>, TList<Long<2>, Nothing>>>);

static_assert(std::is_same_v<
    tlist_remove_t<
        Long<4>,
        TList<Long<0>, TList<Long<1>, TList<Long<2>, TList<Long<3>, TList<Long<4>, Nothing>>>>>
    >,
    TList<Long<0>, TList<Long<1>, TList<Long<2>, TList<Long<3>, Nothing>>>>
>);

#endif

