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

/** At this moment, the function move from puzzle accepts a value and a type. Thats
 * a problem for tlist_map because it forces that the function should accept
 * types. Because of this we create this Mov type that stores a Movement.
 * The function \a func can extract the value and call move with it.
 */
template<Movement M>
using Mov = std::integral_constant<Movement, M>;

template<class Puz, uint64_t Distance, uint64_t Moves, class Previous, class Movement>
struct GameState {
    using puz = Puz;
    static constexpr uint64_t distance = Distance;
    static constexpr uint64_t moves = Moves;
    using previous = Previous;
    using mov = Movement;
};

/**
 * Describes how to create the initial GameState.
 */
template<class Puz>
using initial_GameState = tlist_add_t<GameState<Puz, ManhattanDistance<Puz::value>, 0, Nothing, Nothing>, EmptyList>;

/**
 * Describes the processing list. It is a ordered list with the GameStates to process.
 * The first element should be the GameState nearest to the solution (or the solution).
 *
 * The processin list is created with one element, the initial GameState.
 */
template<class Puz>
using ProcList = tlist_add_t<initial_GameState<Puz>, EmptyList>;

#define LIST0()           EmptyList
#define LIST1(a)          tlist_add_t<a, LIST0()>
#define LIST2(a, b)       tlist_add_t<a, LIST1(b)>
#define LIST3(a, b, c)    tlist_add_t<a, LIST2(b, c)>
#define LIST4(a, b, c, d) tlist_add_t<a, LIST3(b, c, d)>

template<bool B, class move, class GS, class M> struct func_helper {
    using type = GameState<move, ManhattanDistance<move::value>, GS::moves + 1, GS, M>;
};
template<class move, class GS, class M> struct func_helper<true, move, GS, M> : Nothing {};

template<class GS>
struct generate_moves {
    template<class  M>
    struct func { 
        using move = move_t<M::value, typename GS::puz>;

        using type = typename func_helper<is_nothing_v<move>, move, GS, M>::type;
    };

    using type = tlist_map_t<func, LIST4(Mov<Up>, Mov<Right>, Mov<Down>, Mov<Left>)>;
};

template<class Puz>
using generate_moves_t = typename generate_moves<Puz>::type;

template<class State1, class State2>
struct GameStateSortFunc {
    static constexpr bool value = (State1::distance + State1::moves) < (State2::distance + State2::moves);
};

template<class State1>
struct GameStateSortFunc<State1, Nothing> {
    static constexpr bool value = true;
};

/* The heuristic function we will use is the Mnahattan distance of the movement
 * and the length of all the moves we did */
template<class Puz, class Moves>
constexpr uint64_t Heuristic = ManhattanDistance<Puz::value> + tlist_size_v<Moves>;


template<class ProcList>
struct resolv;

template<class ProcList>
using resolv_t = typename resolv<ProcList>::type;

template<bool IsComplete, class ProcList>
struct resolv_helper;
template<bool IsComplete, class ProcList>
using resolv_helper_t = resolv_helper<IsComplete, ProcList>;

template<class ProcList>
struct resolv_helper<true, ProcList> {
    using type = tlist_head_t<ProcList>;
};

template<class ProcList>
struct resolv_helper<false, ProcList> {
    using actGS = tlist_head_t<ProcList>;

    template<class List, class GS>
    using gs_add = tlist_sort_add_unique<GS, List, GameStateSortFunc>;

    using type = resolv_t<
        tlist_foldl_t<
            gs_add,
            tlist_tail_t<ProcList>,
            generate_moves_t<actGS>
        >
    >;
};

template<class ProcList>
struct resolv : resolv_helper<std::is_same_v<typename tlist_head_t<ProcList>::puz, PuzzleResolved>, ProcList> {};


#include <iostream>


constexpr char direction[4] = {
    [Up]    = 'u',
    [Right] = 'r',
    [Down]  = 'd',
    [Left]  = 'l'
};

template<class Path>
struct show_path : show_path<typename Path::previous> {
    show_path() {
        using move = typename Path::mov;
        if constexpr (!is_nothing_v<move>)
           std::cout << direction[move::value];
    }
};

template<>
struct show_path<Nothing> {};

int main() {
    //using to_solve = Long<0x1230'5674'9ab8'defc>;
    //using to_solve = Long<0x5123'9674'dab8'0efc>;
    using to_solve = Long<0x01234'5678'9abc'def>;
    //using to_solve = Long<0x1234'5678'9abc'def0>;
    
    using solution = resolv_t<initial_GameState<to_solve>>;
    show_path<solution>{};
    
    return 0;
}
