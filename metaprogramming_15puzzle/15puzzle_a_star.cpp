#include <type_traits>
#include "puzzle.h"
#include "tlist.h"

/**
 * This is the last state of the puzzle. Its ManhattenDistance is 0.
 */ 
using PuzzleResolved = Long<0x1234'5678'9abc'def0>;

#if (defined USE_DEFINES)

#define abs_sub(A, B) (((A) > (B)) ? ((A) - (B)) : ((B) - (A)))
#define coord_value(Puz) (((Puz & 0xfull) + 15) % 16)
#define coord_x_from_pos(Pos) ((15 - Pos) % 4)
#define coord_y_from_pos(Pos) ((15 - Pos) / 4)
#define dest_coord_x(coord_value) ((coord_value) % 4)
#define dest_coord_y(coord_value) ((coord_value) / 4)

#else
constexpr uint64_t abs_sub(const uint64_t a, const uint64_t b) {
    return (a > b) ? (a - b) : (b - a); 
}

constexpr uint64_t coord_value(const uint64_t puz) {
// Because we are using unsigned values, we can remove 1 to the value. Adding 15 and
// the doing the modulo we get the same result: substract one to all the values
// but 0 is converted in 15. In other words, we want the distance of each value to
// this configuration: 123456789abcdef0.
    return ((puz & 0xfull) + 15) % 16;
}
constexpr uint64_t coord_x_from_pos(const uint64_t pos) {
    return (15 - pos) % 4;
}

constexpr uint64_t coord_y_from_pos(const uint64_t pos) {
    return (15 - pos) / 4;
}

constexpr uint64_t dest_coord_x(const uint64_t coord_value) {
    return coord_value % 4;
}

constexpr uint64_t dest_coord_y(const uint64_t coord_value) {
    return coord_value / 4;
}
#endif



template<uint64_t Puz, uint64_t Pos>
constexpr uint64_t ManhattanDistance_helper = 
        /* Number of horizontal movements for the current piece to go to its position */
        abs_sub(dest_coord_x(coord_value(Puz)), coord_x_from_pos(Pos)) +
        /* Number of vertical movements for the current piece to go to its position */
        abs_sub(dest_coord_y(coord_value(Puz)), coord_y_from_pos(Pos)) +
        /* The sum of the Manhattan Dstance of the rest of pieces */
        ManhattanDistance_helper<(Puz >> 4), Pos + 1>;

template<uint64_t Puz>
constexpr uint64_t ManhattanDistance_helper<Puz, 16> = 0;

template<uint64_t Puz>
constexpr uint64_t ManhattanDistance = ManhattanDistance_helper<Puz, 0>;

static_assert(ManhattanDistance<0x1234'5678'9abc'def0> == 0);
static_assert(ManhattanDistance<0x1234'5678'9abc'de0f> == 2);
static_assert(ManhattanDistance<0x1234'5678'9ab0'defc> == 2);
static_assert(ManhattanDistance<0x1234'5670'9ab8'defc> == 4);
static_assert(ManhattanDistance<0x1234'5607'9ab8'defc> == 6);

/* The heuristic function we will use is the Mnahattan distance of the movement
 * and the length of all the moves we did */
template<class Puz, class Moves>
constexpr uint64_t Heuristic = ManhattanDistance<Puz::value> + tlist_size_v<Moves>;

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


constexpr char direction[4] = {
    [Up]    = 'u',
    [Right] = 'r',
    [Down]  = 'd',
    [Left]  = 'l'
};

template<class Path>
struct show_path : show_path<typename Path::next> {
    show_path() {
        std::cout << direction[Path::type::direction];
    }
};

template<>
struct show_path<Nothing> {};

int main() {
    //using to_solve = Long<0x1230'5674'9ab8'defc>;
    using to_solve = Long<0x5123'9674'dab8'0efc>;
    //using to_solve = Long<0x01234'5678'9abc'def>;
    //using to_solve = Long<0x1234'5678'9abc'def0>;

    if constexpr (std::is_same_v<resolv_t<to_solve>, Nothing>) {
        std::cout << "" << "hasn't got result" << std::endl;
    }
    else {
        show_path<resolv_t<to_solve>>{};
        std::cout << std::endl;
    }

    return 0;
}
