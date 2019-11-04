#include <type_traits>
#include <cstdint>

/**
* When an action (structs with its name in lower case), cannot do its operation,
* it returns Nothing.
* All actions returns a 'type'.
*/
struct Nothing {};

template<uint64_t P>
using Long = std::integral_constant<uint64_t, P>;

/**
 * Kind of movements the hole can do
 */
enum Movement: long {
    Up,
    Right,
    Down,
    Left
};

template<uint64_t Puz, uint64_t Pos>
struct __get_hole_position {
    using type = std::conditional_t<
        (Puz & 0xf) == 0,
        Long<Pos>,
        typename __get_hole_position<(Puz >> 4), Pos + 1>::type
    >;
};

template<uint64_t Puz>
struct __get_hole_position<Puz, 16> {
    using type = Nothing;
};

template<class MaybePuzzle>
struct get_hole_position {
    using type = typename __get_hole_position<MaybePuzzle::value, 0>::type;
};

template<>
struct get_hole_position<Nothing> {
    using type = Nothing;
};

template<class MaybePuzzle>
using get_hole_position_t = typename get_hole_position<MaybePuzzle>::type;

static_assert(std::is_same_v<get_hole_position_t<Nothing>, Nothing>);
static_assert(std::is_same_v<get_hole_position_t<Long<0x123456789abcdef0>>, Long<0>>);
static_assert(std::is_same_v<get_hole_position_t<Long<0x123456789abcde0f>>, Long<1>>);
static_assert(std::is_same_v<get_hole_position_t<Long<0x123456789abcd0ef>>, Long<2>>);
static_assert(std::is_same_v<get_hole_position_t<Long<0x123456789abc0def>>, Long<3>>);
static_assert(std::is_same_v<get_hole_position_t<Long<0x123456789ab0cdef>>, Long<4>>);
static_assert(std::is_same_v<get_hole_position_t<Long<0x123456789a0bcdef>>, Long<5>>);
static_assert(std::is_same_v<get_hole_position_t<Long<0x1234567890abcdef>>, Long<6>>);
static_assert(std::is_same_v<get_hole_position_t<Long<0x1234567809abcdef>>, Long<7>>);
static_assert(std::is_same_v<get_hole_position_t<Long<0x1234567089abcdef>>, Long<8>>);
static_assert(std::is_same_v<get_hole_position_t<Long<0x1234560789abcdef>>, Long<9>>);
static_assert(std::is_same_v<get_hole_position_t<Long<0x1234506789abcdef>>, Long<10>>);
static_assert(std::is_same_v<get_hole_position_t<Long<0x1234056789abcdef>>, Long<11>>);
static_assert(std::is_same_v<get_hole_position_t<Long<0x1230456789abcdef>>, Long<12>>);
static_assert(std::is_same_v<get_hole_position_t<Long<0x1203456789abcdef>>, Long<13>>);
static_assert(std::is_same_v<get_hole_position_t<Long<0x1023456789abcdef>>, Long<14>>);
static_assert(std::is_same_v<get_hole_position_t<Long<0x0123456789abcdef>>, Long<15>>);
static_assert(std::is_same_v<get_hole_position_t<Long<0xf123456789abcdef>>, Nothing>);

template<bool Condition, uint64_t Pos, Movement M, uint64_t Puz>
struct __move;

template<uint64_t Pos, uint64_t Puz>
struct __move<true, Pos, Up, Puz> {
    using type = Long<
        (Puz & ~(0xfull << (4 * (Pos + 4)))) |
        ((Puz &  (0xfull << (4 * (Pos + 4)))) >> 16)
    >;
};
template<uint64_t Pos, uint64_t Puz>
struct __move<true, Pos, Right, Puz> {
    using type = Long<
        (Puz & ~(0xfull << (4 * (Pos - 1)))) |
        ((Puz &  (0xfull << (4 * (Pos - 1)))) << 4)
    >;
};
template<uint64_t Pos, uint64_t Puz>
struct __move<true, Pos, Down, Puz> {
    using type = Long<
        (Puz & ~(0xfull << (4 * (Pos - 4)))) |
        ((Puz &  (0xfull << (4 * (Pos - 4)))) << 16)
    >;    
};
template<uint64_t Pos, uint64_t Puz>
struct __move<true, Pos, Left, Puz> {
    using type = Long<
        (Puz & ~(0xfull << (4 * (Pos + 1)))) |
        ((Puz &  (0xfull << (4 * (Pos + 1)))) >> 4)
    >;    
};

template<uint64_t Pos, Movement M, uint64_t Puz>
struct __move<false, Pos, M, Puz> {
    using type = Nothing;
};

template<bool Condition, uint64_t Pos, Movement M, uint64_t Puz>
using __move_t = typename __move<Condition, Pos, M, Puz>::type;

static_assert(std::is_same_v<__move_t<false, 0, Up, 0>, Nothing>);

template<Movement M, class MaybePuzzle>
struct move {
    using type = 
    std::conditional_t<M == Up,    __move_t<get_hole_position_t<MaybePuzzle>::value / 4 != 3, get_hole_position_t<MaybePuzzle>::value, Up   , MaybePuzzle::value>,
    std::conditional_t<M == Right, __move_t<get_hole_position_t<MaybePuzzle>::value % 4 != 0, get_hole_position_t<MaybePuzzle>::value, Right, MaybePuzzle::value>,
    std::conditional_t<M == Down,  __move_t<get_hole_position_t<MaybePuzzle>::value / 4 != 0, get_hole_position_t<MaybePuzzle>::value, Down , MaybePuzzle::value>,
                     /*M == Left*/ __move_t<get_hole_position_t<MaybePuzzle>::value % 4 != 3, get_hole_position_t<MaybePuzzle>::value, Left , MaybePuzzle::value>
    > > >;
    static constexpr Movement direction = M;
};

/* Try to move nothing results nothing */
template<Movement M>
struct move<M, Nothing> {
    using type = Nothing;
};

template<Movement M, class MaybePuzzle>
using move_t = typename move<M, MaybePuzzle>::type;

/* Move nothing results in nothing */
static_assert(std::is_same_v<move_t<Up   , Nothing>, Nothing>);
static_assert(std::is_same_v<move_t<Right, Nothing>, Nothing>);
static_assert(std::is_same_v<move_t<Down , Nothing>, Nothing>);
static_assert(std::is_same_v<move_t<Left , Nothing>, Nothing>);
/* trying move corners */
/* bottom right */
static_assert(std::is_same_v<move_t<Up   , Long<0x1234'5678'9abc'def0>>, Long<0x1234'5678'9ab0'defc>>);
static_assert(std::is_same_v<move_t<Right, Long<0x1234'5678'9abc'def0>>, Nothing>);
static_assert(std::is_same_v<move_t<Down , Long<0x1234'5678'9abc'def0>>, Nothing>);
static_assert(std::is_same_v<move_t<Left , Long<0x1234'5678'9abc'def0>>, Long<0x1234'5678'9abc'de0f>>);
/* bottom left */
static_assert(std::is_same_v<move_t<Up   , Long<0x1234'5678'9abc'0def>>, Long<0x1234'5678'0abc'9def>>);
static_assert(std::is_same_v<move_t<Right, Long<0x1234'5678'9abc'0def>>, Long<0x1234'5678'9abc'd0ef>>);
static_assert(std::is_same_v<move_t<Down , Long<0x1234'5678'9abc'0def>>, Nothing>);
static_assert(std::is_same_v<move_t<Left , Long<0x1234'5678'9abc'0def>>, Nothing>);
/* upper left */
static_assert(std::is_same_v<move_t<Up   , Long<0x0123'4567'89ab'cdef>>, Nothing>);
static_assert(std::is_same_v<move_t<Right, Long<0x0123'4567'89ab'cdef>>, Long<0x1023'4567'89ab'cdef>>);
static_assert(std::is_same_v<move_t<Down , Long<0x0123'4567'89ab'cdef>>, Long<0x4123'0567'89ab'cdef>>);
static_assert(std::is_same_v<move_t<Left , Long<0x0123'4567'89ab'cdef>>, Nothing>);
/* upper right */
static_assert(std::is_same_v<move_t<Up   , Long<0x1230'4567'89ab'cdef>>, Nothing>);
static_assert(std::is_same_v<move_t<Right, Long<0x1230'4567'89ab'cdef>>, Nothing>);
static_assert(std::is_same_v<move_t<Down , Long<0x1230'4567'89ab'cdef>>, Long<0x1237'4560'89ab'cdef>>);
static_assert(std::is_same_v<move_t<Left , Long<0x1230'4567'89ab'cdef>>, Long<0x1203'4567'89ab'cdef>>);
/* trying a element in the middle */
static_assert(std::is_same_v<move_t<Up   , Long<0x1234'5067'89ab'cdef>>, Long<0x1034'5267'89ab'cdef>>);
static_assert(std::is_same_v<move_t<Right, Long<0x1234'5067'89ab'cdef>>, Long<0x1234'5607'89ab'cdef>>);
static_assert(std::is_same_v<move_t<Down , Long<0x1234'5067'89ab'cdef>>, Long<0x1234'5967'80ab'cdef>>);
static_assert(std::is_same_v<move_t<Left , Long<0x1234'5067'89ab'cdef>>, Long<0x1234'0567'89ab'cdef>>);

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

template<class Type, class List>
struct tlist_add_unique {
    using type = std::conditional_t<
        tlist_has_element<Type, List>::type::value,
        List,
        TList<Type, List>
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

/**
 * Helper to solve a puzzle. It receive the actual puzzle
 * to solve, the movements done to get to this puzzle and
 * all the visited positions.
 *
 * All the __resolv types should have:
 *    * type: a type indicating it is a dead end path. In this case, type is Nothing.
 *    * moves: the list of moves used to get to the actual puzzle.
 *    * visited: all the puzzles visited during the resolution.
 */
template<class MaybePuzzle, class MoveList, class Visited>
struct __resolv;

template<class MaybePuzzle, class MoveList, class Visited>
using __resolv_t = typename __resolv<MaybePuzzle, MoveList, Visited>::type;

template<class MaybePuzzle, class MoveList, class Visited>
using __resolv_visited = typename __resolv<MaybePuzzle, MoveList, Visited>::visited;

template<bool Choose, Movement M, class MaybePuzzle, class MoveList, class Visited, class Result, template<class, class, class> class Action>
struct __resolv_move_helper_selector {
    using type = Action<
            move_t<M, MaybePuzzle>,
            tlist_add_unique_t<
                move<M, MaybePuzzle>,
                MoveList
            >,
            tlist_add_unique_t<
                move_t<M, MaybePuzzle>,
                Visited
            >
        >;
};

template<Movement M, class MaybePuzzle, class MoveList, class Visited, class Result, template<class, class, class> class Action>
struct __resolv_move_helper_selector<true, M, MaybePuzzle, MoveList, Visited, Result, Action> {
    using type = Result;
};
template<bool Choose, Movement M, class MaybePuzzle, class MoveList, class Visited, class Result, template<class, class, class> class Action>
using __resolv_move_helper_selector_t = typename __resolv_move_helper_selector<Choose, M, MaybePuzzle, MoveList, Visited, Result, Action>::type;

template<Movement M, class MaybePuzzle, class MoveList, class Visited, class Result, template<class, class, class> class Action>
using __resolv_move_helper = __resolv_move_helper_selector_t<
        std::is_same_v<move_t<M, MaybePuzzle>, Nothing> || tlist_has_element_v<move_t<M, MaybePuzzle>, Visited>,
        M, MaybePuzzle, MoveList, Visited, Result, Action
    >;

template<Movement M, class MaybePuzzle, class MoveList, class Visited>
struct __resolv_move {
    using type    = __resolv_move_helper<M, MaybePuzzle, MoveList, Visited, Nothing , __resolv_t      >;
    using visited = __resolv_move_helper<M, MaybePuzzle, MoveList, Visited, Visited , __resolv_visited>;
};
template<Movement M, class MoveList, class Visited>
struct __resolv_move<M, Nothing, MoveList, Visited> {
    using type    = Nothing;
    using visited = Visited;
};

template<Movement M, class MaybePuzzle, class MoveList, class Visited>
using __resolv_move_t = typename __resolv_move<M, MaybePuzzle, MoveList, Visited>::type;

template<Movement M, class MaybePuzzle, class MoveList, class Visited>
using __resolv_move_visited = typename __resolv_move<M, MaybePuzzle, MoveList, Visited>::visited;

template<bool Solved, class MaybePuzzle, class MoveList, class Visited>
struct __resolv_left {
    using type = __resolv_move<Left, MaybePuzzle, MoveList, Visited>;
};
template<class MaybePuzzle, class MoveList, class Visited>
struct __resolv_left<true, MaybePuzzle, MoveList, Visited> {
    using type = Nothing;
};

template<bool Solved, class MaybePuzzle, class MoveList, class Visited>
struct __resolv_down {
    using type = __resolv_move<Down, MaybePuzzle, MoveList, Visited>;
};
template<class MaybePuzzle, class MoveList, class Visited>
struct __resolv_down<true, MaybePuzzle, MoveList, Visited> {
    using type = typename __resolv_left<
        std::is_same_v<move_t<Left, MaybePuzzle>, Nothing> || tlist_has_element_v<move_t<Left, MaybePuzzle>, Visited>,
        MaybePuzzle,
        MoveList,
        __resolv_move_visited<Down, MaybePuzzle, MoveList, Visited>
    >::type;
};

template<bool Solved, class MaybePuzzle, class MoveList, class Visited>
struct __resolv_right {
    using type = __resolv_move<Right, MaybePuzzle, MoveList, Visited>;
};
template<class MaybePuzzle, class MoveList, class Visited>
struct __resolv_right<true, MaybePuzzle, MoveList, Visited> {
    using type = typename __resolv_down<
        std::is_same_v<move_t<Down, MaybePuzzle>, Nothing> || tlist_has_element_v<move_t<Down, MaybePuzzle>, Visited>,
        MaybePuzzle,
        MoveList,
        __resolv_move_visited<Right, MaybePuzzle, MoveList, Visited>
    >::type;
};

template<bool Unsolved, class MaybePuzzle, class MoveList, class Visited>
struct __resolv_up {
    using type = __resolv_move<Up, MaybePuzzle, MoveList, Visited>;
};

template<class MaybePuzzle, class MoveList, class Visited>
struct __resolv_up<true, MaybePuzzle, MoveList, Visited> {
    using type = typename __resolv_right<
        std::is_same_v<move_t<Right, MaybePuzzle>, Nothing> || tlist_has_element_v<move_t<Right, MaybePuzzle>, Visited>,
        MaybePuzzle,
        MoveList,
        __resolv_move_visited<Up, MaybePuzzle, MoveList, Visited>
    >::type;
};

template<class MaybePuzzle, class MoveList, class Visited>
struct __resolv {
    using type_temp = typename __resolv_up<
        std::is_same_v<move_t<Up, MaybePuzzle>, Nothing> || tlist_has_element_v<move_t<Up, MaybePuzzle>, Visited>,
        MaybePuzzle,
        MoveList,
        Visited
    >::type;

    using type = typename type_temp::type;
    using visited = typename type_temp::visited;
};


template<class MoveList, class Visited>
struct __resolv<Nothing, MoveList, Visited> {
    using type = Nothing;
    using visited = Visited;
};

template<class MoveList, class Visited>
struct __resolv<Long<0x1234'5678'9abc'def0>, MoveList, Visited> {
    using type = MoveList;
    using visited = Visited;
};

/* https://stackoverflow.com/questions/40781817/heuristic-function-for-solving-weighted-15-puzzle */
template<class MaybePuzzle>
struct resolv : __resolv<MaybePuzzle, Nothing, tlist_add_unique_t<MaybePuzzle, Nothing>> {};

template<>
struct resolv<Nothing> {
    using type = Nothing;
};

template<class MaybePuzzle>
using resolv_t = typename resolv<MaybePuzzle>::type;

template<class MaybePuzzle>
using resolv_moves = typename resolv<MaybePuzzle>::moves;

#include <iostream>


template<class Path>
struct __show_path {
    __show_path() {
        __show_path<typename Path::next>{};

        if constexpr (Path::type::direction == Up) {
            std::cout << "u";
        }
        else if constexpr (Path::type::direction == Right) {
            std::cout << "r";
        }
        else if constexpr (Path::type::direction == Down) {
            std::cout << "d";
        }
        else if constexpr (Path::type::direction == Left) {
            std::cout << "l";
        }
    }
};

template<>
struct __show_path<Nothing> {};

int main() {
    //using to_solve = Long<0x1230'5674'9ab8'defc>;
    using to_solve = Long<0x5123'9674'dab8'0efc>;
    //using to_solve = Long<0x01234'5678'9abc'def>;
    //using to_solve = Long<0x1234'5678'9abc'def0>;

    if constexpr (std::is_same_v<resolv_t<to_solve>, Nothing>) {
        std::cout << "" << "hasn't got result" << std::endl;
    }
    else {
        __show_path<resolv_t<to_solve>>{};
        std::cout << std::endl;
    }
    return 0;
}