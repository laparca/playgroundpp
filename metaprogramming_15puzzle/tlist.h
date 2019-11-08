#ifndef TLIST_H
#define TLIST_H

#include <type_traits>
#include "nothing.h"

/* List that stores types in compilation time. A empty list is Nothing */
template<class Type, class Next>
struct TList {
    using type = Type;
    using next = Next;
};

using EmptyList = Nothing;

/**
 * Checks if the type \a Type is in list \a List.
 */
template<class Type, class List>
struct tlist_has_element;

template<class Type, class List>
inline constexpr bool tlist_has_element_v = tlist_has_element<Type, List>::value;

/**
 * Returns the number of elements of the list \a List.
 */
template<class List>
struct tlist_size;

template<class List>
inline constexpr uint64_t tlist_size_v = tlist_size<List>::value;

/**
 * Returns the first element of the list \a List.
 */
template<class List>
struct tlist_head;

template<class List>
using tlist_head_t = typename tlist_head<List>::type;

/**
 * Returns the tail of the list \a List. The tail is the list
 * without the first element (the head).
 */
template<class List>
struct tlist_tail;

template<class List>
using tlist_tail_t = typename tlist_tail<List>::type;

/**
 * Returns the element of the list \a List at position \a P.
 */
template<class List, int P>
struct tlist_get;

template<class List, int P>
using tlist_get_t = typename tlist_get<List, P>::type;

/**
 * Adds the type \a Type at the top of the list \a List.
 */
template<class Type, class List>
struct tlist_add;

template<class Type, class List>
using tlist_add_t = typename tlist_add<Type, List>::type;

/**
 * Adds the type \a Type to the list \a List if it isn't in the list.
 */
template<class Type, class List>
struct tlist_add_unique;

template<class Type, class List>
using tlist_add_unique_t = typename tlist_add_unique<Type, List>::type;

/**
 * Add the type \a Type in the list \a List sorting it by \a SortCmp.
 * The position of \a Type is when \a SortCmp returns true. \a SortCmp checks
 * if \a Type go before the checked elemento of the list.
 */
template<typename Type, class List, template<typename, typename> typename SortCmp>
struct tlist_sort_add;

template<typename Type, class List, template<typename, typename> typename SortCmp>
using tlist_sort_add_t = typename tlist_sort_add<Type, List, SortCmp>::type;

/**
 * Like \a tlist_sort_add, but \a Type couldn't be duplicated.
 */
template<class Type, class List, template<typename, typename> typename SortCmp>
struct tlist_sort_add_unique;

template<typename Type, class List, template<typename, typename> typename SortCmp>
using tlist_sort_add_unique_t = typename tlist_sort_add_unique<Type, List, SortCmp>::type;

/**
 * Sorts the list \a List using \a SortCmp.
 */
template<class List, template<typename, typename> typename SortCmp>
struct tlist_sort_by;

template<class List, template<typename, typename> typename SortCmp>
using tlist_sort_by_t = typename tlist_sort_by<List, SortCmp>::type;

/**
 * Removes the first element of the list \a List that satisfies \a If.
 */
template<class List, template<class> typename If>
struct tlist_remove_if;

template<class List, template<class> typename If>
using tlist_remove_if_t = typename tlist_remove_if<List, If>::type;

/**
 * Removes first element of the type \a T of the list \a List.
 */
template<class T, class List>
struct tlist_remove;

template<class T, class List>
using tlist_remove_t = typename tlist_remove<T, List>::type;

/**
 * Removes all the elements of the list \a List that satisfy \a If.
 */
template<class List, template<class> typename If>
struct tlist_remove_all_if;

template<class List, template<class> typename If>
using tlist_remove_all_if_t = typename tlist_remove_all_if<List, If>::type;

/**
 * Concatenates the list \a List1 and the list \a List2.
 */
template<class List1, class List2>
struct tlist_concat;

template<class List1, class List2>
using tlist_concat_t = typename tlist_concat<List1, List2>::type;

/**
 * Returns a new list where each element is the element is the result
 * of run \a Func in the corresponding element of \a List.
 */
template<template<class>typename Func, class List>
struct tlist_map;

template<template<class>typename Func, class List>
using tlist_map_t = typename tlist_map<Func, List>::type;

/**
 * Fold operation. It calls Func<Initial, head> recursively and returst the
 * last Initial calculated.
 */
template<template<class, class> class Func, class Initial, class List>
struct tlist_foldl;

template<template<class, class> class Func, class Initial, class List>
using tlist_foldl_t = typename tlist_foldl<Func, Initial, List>::type;

template<template<class, class> class Func, class List, class Initial>
struct tlist_foldr;

template<template<class, class> class Func, class List, class Initial>
using tlist_foldr_t = typename tlist_foldr<Func, List, Initial>::type;

template<typename List>
struct tlist_reverse;

template<typename List>
using tlist_reverse_t = typename tlist_reverse<List>::type;

/* IMPLEMENTATION SECTION */


template<class Type, class List>
struct tlist_has_element : std::conditional_t<
        std::is_same_v<Type, typename List::type>,
        std::true_type,
        typename tlist_has_element<Type, typename List::next>::type
    > {};

template<class Type>
struct tlist_has_element<Type, EmptyList> : std::false_type {};

static_assert(tlist_has_element_v<Long<0>, EmptyList> == false);
static_assert(tlist_has_element_v<Long<0>, TList<Long<0>, EmptyList>> == true);
static_assert(tlist_has_element_v<Long<0>, TList<Long<0>, TList<Long<1>, EmptyList>>> == true);
static_assert(tlist_has_element_v<Long<0>, TList<Long<1>, TList<Long<0>, EmptyList>>> == true);
static_assert(tlist_has_element_v<Long<0>, TList<Long<1>, TList<Long<2>, EmptyList>>> == false);
static_assert(tlist_has_element_v<move_t<Right, Long<0x1234'5678'9abc'de0f>>, EmptyList> == false);
static_assert(tlist_has_element_v<move_t<Right, Long<0x1234'5678'9abc'de0f>>, TList<Long<0x1234'5678'9abc'def0>, EmptyList>> == true);

template<class List>
struct tlist_size {
    static constexpr uint64_t value = 1 + tlist_size<typename List::next>::value;
};

template<>
struct tlist_size<EmptyList> {
    static constexpr uint64_t value = 0;
};

static_assert(tlist_size_v<EmptyList> == 0);
static_assert(tlist_size_v<TList<Long<0>, EmptyList>> == 1);
static_assert(tlist_size_v<TList<Long<1>, TList<Long<0>, EmptyList>>> == 2);

template<class List>
struct tlist_head {
    using type = typename List::type;
};

template<class List>
struct tlist_tail {
    using type = typename List::next;
};

template<class List, int P>
struct tlist_get : tlist_get<tlist_tail_t<List>, P-1> {};

template<class List>
struct tlist_get<List, 0> : List {};

template<int P>
struct tlist_get<EmptyList, P> : EmptyList {};

template<>
struct tlist_get<EmptyList, 0> : EmptyList {};

template<class Type, class List>
struct tlist_add {
    using type = TList<Type, List>;
};

template<class List>
struct tlist_add<Nothing, List> {
    using type = List;
};

template<class Type, class List>
struct tlist_add_unique {
    using type = std::conditional_t<
        tlist_has_element<Type, List>::type::value,
        List,
        tlist_add_t<Type, List>
    >;
};

template<class List>
struct tlist_add_unique<EmptyList, List> {
    using type = List;
};

static_assert(std::is_same_v<tlist_add_unique_t<Long<0>, EmptyList>, TList<Long<0>, EmptyList>>);
static_assert(std::is_same_v<tlist_add_unique_t<Long<1>, TList<Long<0>, EmptyList>>, TList<Long<1>, TList<Long<0>, EmptyList>>>);
static_assert(std::is_same_v<tlist_add_unique_t<Long<0>, TList<Long<0>, EmptyList>>, TList<Long<0>, EmptyList>>);

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
struct tlist_sort_add_unique_helper : tlist_sort_add<Type, List, SortCmp> {};

template<class Type, class List, template<typename, typename> typename SortCmp>
struct tlist_sort_add_unique_helper<true, Type, List, SortCmp> {
   using type = List;
};

template<class Type, class List, template<class, class> typename SortCmp>
struct tlist_sort_add_unique : tlist_sort_add_unique_helper<tlist_has_element_v<Type, List>, Type, List, SortCmp> {};

template<class List, template<class, class> typename SortCmp>
struct tlist_sort_add_unique<Nothing, List, SortCmp> {
    using type = List;
};

template<typename A, typename B>
struct great_than {
    static constexpr bool value = A::value > B::value;
};
template<typename A>
struct great_than<A, EmptyList> {
    static constexpr bool value = true;
};

template<typename A, typename B>
struct less_than {
    static constexpr bool value = A::value < B::value;
};
template<typename A>
struct less_than<A, EmptyList> {
    static constexpr bool value = true;
};

static_assert(
    std::is_same_v<
        tlist_sort_add_t<
            Long<0>,
            EmptyList,
            great_than
        >,
        TList<Long<0>, EmptyList>
    >
);
static_assert(
    std::is_same_v<
        tlist_sort_add_t<
            Long<0>,
            TList<Long<1>, EmptyList>,
            great_than
        >,
        TList<Long<1>, TList<Long<0>, EmptyList>>
    >
);
static_assert(
    std::is_same_v<
        tlist_sort_add_t<
            Long<0>,
            TList<Long<2>, TList<Long<1>, EmptyList>>,
            great_than
        >,
        TList<Long<2>, TList<Long<1>, TList<Long<0>, EmptyList>>>
    >
);
static_assert(
    std::is_same_v<
        tlist_sort_add_t<
            Long<0>,
            TList<Long<1>, TList<Long<2>, EmptyList>>,
            less_than
        >,
        TList<Long<0>, TList<Long<1>, TList<Long<2>, EmptyList>>>
    >
);

/**
 * Implements bubble sort. Yes, it's aweful, but easy
 */
template<class SortedList, class UnsortedList, template<typename, typename> typename SortCmp>
struct tlist_sort_by_helper : tlist_sort_by_helper<tlist_sort_add_t<tlist_head_t<UnsortedList>, SortedList, SortCmp>, tlist_tail_t<UnsortedList>, SortCmp> {};

template<class SortedList, template<typename, typename> typename SortCmp>
struct tlist_sort_by_helper<SortedList, EmptyList, SortCmp> {
    using type = SortedList;
};

template<class List, template<typename, typename> typename SortCmp>
struct tlist_sort_by : tlist_sort_by_helper<EmptyList, List, SortCmp> {};

static_assert(
    std::is_same_v<
        tlist_sort_by_t<
            TList<Long<2>, TList<Long<0>, TList<Long<3>, TList<Long<1>, EmptyList>>>>, 
            less_than
        >,
        TList<Long<0>, TList<Long<1>, TList<Long<2>, TList<Long<3>, EmptyList>>>>
    >
);

template<bool IsSameThanHead, class List, template<typename> typename If>
struct tlist_remove_if_helper : tlist_add<
    tlist_head_t<List>,
    tlist_remove_if_t<tlist_tail_t<List>, If>
> {};

template<class List, template<class> typename If>
struct tlist_remove_if_helper<true, List, If> : tlist_tail<List> {};

template<bool IsSameThanHead, template<class> typename If>
struct tlist_remove_if_helper<IsSameThanHead, EmptyList, If> : EmptyList {};


template<class List, template<class> typename If>
struct tlist_remove_if : tlist_remove_if_helper<If<tlist_head_t<List>>::value, List, If> {};

template<template<class>typename If>
struct tlist_remove_if<EmptyList, If> : EmptyList {};

template<class T, class List>
struct tlist_remove {
    template<typename Other>
    using If = std::is_same<T, Other>;
    using type = tlist_remove_if_t<List, If>;
};

static_assert(std::is_same_v<tlist_remove_t<Long<0>, EmptyList>, EmptyList>);
static_assert(std::is_same_v<tlist_remove_t<Long<0>, TList<Long<0>, EmptyList>>, EmptyList>);
static_assert(std::is_same_v<tlist_remove_t<Long<0>, TList<Long<0>, TList<Long<1>, EmptyList>>>, TList<Long<1>, EmptyList>>);
static_assert(std::is_same_v<tlist_remove_t<Long<0>, TList<Long<1>, TList<Long<0>, EmptyList>>>, TList<Long<1>, EmptyList>>);

static_assert(std::is_same_v<tlist_remove_t<Long<0>, TList<Long<2>, TList<Long<0>, TList<Long<1>, EmptyList>>>>, TList<Long<2>, TList<Long<1>, EmptyList>>>);
static_assert(std::is_same_v<tlist_remove_t<Long<0>, TList<Long<1>, TList<Long<2>, EmptyList>>>, TList<Long<1>, TList<Long<2>, EmptyList>>>);

static_assert(std::is_same_v<
    tlist_remove_t<
        Long<4>,
        TList<Long<0>, TList<Long<1>, TList<Long<2>, TList<Long<3>, TList<Long<4>, EmptyList>>>>>
    >,
    TList<Long<0>, TList<Long<1>, TList<Long<2>, TList<Long<3>, EmptyList>>>>
>);

template<bool ShouldBeRemove, class List, template<class> typename If>
struct tlist_remove_all_if_helper;

template<bool ShouldBeRemove, class List, template<class> typename If>
using tlist_remove_all_if_helper_t = tlist_remove_all_if_helper<ShouldBeRemove, List, If>;

template<bool ShouldBeRemove, class List, template<class> typename If>
struct tlist_remove_all_if_helper : tlist_add<tlist_head_t<List>, tlist_remove_all_if_helper_t<If<tlist_get_t<List, 1>>::value, tlist_tail_t<List>, If>> {};

template<class List, template<class> typename If>
struct tlist_remove_all_if_helper<true, List, If> : tlist_remove_all_if_helper<If<tlist_get_t<List, 1>>::value, tlist_tail_t<List>, If> {};

template<bool ShouldBeRemove, template<class>typename If>
struct tlist_remove_all_if_helper<ShouldBeRemove, EmptyList, If> : EmptyList {};

template<class List, template<class> typename If>
struct tlist_remove_all_if : tlist_remove_all_if_helper<If<tlist_head_t<List>>::value, List, If> {};

template<class List1, class List2>
struct tlist_concat : tlist_add<tlist_head_t<List1>, typename tlist_concat<tlist_tail_t<List1>, List2>::type> {};

template<class List2>
struct tlist_concat<EmptyList, List2> {
    using type = List2;
};

static_assert(std::is_same_v<tlist_concat_t<TList<Long<0>, EmptyList>, TList<Long<1>, EmptyList>>, TList<Long<0>, TList<Long<1>, EmptyList>>>);
static_assert(std::is_same_v<
    tlist_concat_t<
        TList<
            Long<0>,
            TList<
                Long<1>,
                EmptyList
            >
        >,
        TList<
            Long<2>,
            TList<
                Long<3>,
                EmptyList
            >
        >
    >,
    TList<
        Long<0>,
        TList<
            Long<1>,
            TList<
                Long<2>,
                TList<
                    Long<3>,
                    EmptyList
                >
            >
        >
    >
>);

template<template<class>typename Func, class List>
struct tlist_map : tlist_add<typename Func<tlist_head_t<List>>::type, typename tlist_map<Func, tlist_tail_t<List>>::type> {};

template<template<class>typename Func>
struct tlist_map<Func, EmptyList> : EmptyList {};

template<class L>
using pow2 = Long<L::value*L::value>;

static_assert(std::is_same_v<
    tlist_map_t<pow2, TList<Long<2>, EmptyList>>,
    TList<Long<4>, EmptyList>
>);

static_assert(std::is_same_v<
    tlist_map_t<pow2, TList<Long<3>, TList<Long<2>, EmptyList>>>,
    TList<Long<9>, TList<Long<4>, EmptyList>>
>);

template<template<class, class> class Func, class Initial, class List>
struct tlist_foldl : tlist_foldl<Func, typename Func<Initial, tlist_head_t<List>>::type, tlist_tail_t<List>> {};

template<template<class, class> class Func, class Initial>
struct tlist_foldl<Func, Initial, EmptyList> {
    using type = Initial;
};

template<template<class, class>class Func, class List, class Initial>
struct tlist_foldr : Func<tlist_head_t<List>, tlist_foldr_t<Func, tlist_tail_t<List>, Initial>> {};

template<template<class, class>class Func, class Initial>
struct tlist_foldr<Func, EmptyList, Initial> {
    using type = Initial;
};

template<template<class, class> class Func>
struct flip {
    template<class A, class B>
    using type = Func<B, A>;
};

template<class List>
struct tlist_reverse : tlist_foldl<flip<tlist_add>::type, EmptyList, List> {};

static_assert(std::is_same_v<
    tlist_reverse_t<TList<Long<0>, TList<Long<1>, TList<Long<2>, EmptyList>>>>,
    TList<Long<2>, TList<Long<1>, TList<Long<0>, EmptyList>>>
>);

#endif

