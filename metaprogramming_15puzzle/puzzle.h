#ifndef PUZZLE_H
#define PUZZLE_H

#include <cstdint>
#include <type_traits>

/* When an action (structs with its name in lower case), cannot do its operation,
 * it returns Nothing.
 * All actions returns a 'type'.
 */
#include "nothing.h"

/* We use Long as representation of the 15 puzzle. It can store as a 64bit value
 * a whole puzzle where the 0 represents the hole. A puzzle like:
 *
 *                     +--+--+--+--+
 *                     | 3|10|  | 6|
 *                     +--+--+--+--+
 *                     | 9| 1|15| 2|
 *                     +--+--+--+--+
 *                     | 7| 5| 4| 8|
 *                     +--+--+--+--+
 *                     |13|14|12|11|
 *                     +--+--+--+--+
 *
 * Can be represented as 0x3a06'91f2'7548'decb. For the purpose of this
 * experiment, we will use Long<0x3a06'91f2'7548'decb>.
 */
#include "long.h"

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
struct move;

template<class MaybePuzzle>
struct move<Up, MaybePuzzle> {
    using type = __move_t<get_hole_position_t<MaybePuzzle>::value / 4 != 3, get_hole_position_t<MaybePuzzle>::value, Up   , MaybePuzzle::value>;
    static constexpr Movement direction = Up;
};
template<class MaybePuzzle>
struct move<Right, MaybePuzzle> {
    using type = __move_t<get_hole_position_t<MaybePuzzle>::value % 4 != 0, get_hole_position_t<MaybePuzzle>::value, Right, MaybePuzzle::value>;
    static constexpr Movement direction = Right;
};
template<class MaybePuzzle>
struct move<Down, MaybePuzzle> {
    using type = __move_t<get_hole_position_t<MaybePuzzle>::value / 4 != 0, get_hole_position_t<MaybePuzzle>::value, Down , MaybePuzzle::value>;
    static constexpr Movement direction = Down;
};
template<class MaybePuzzle>
struct move<Left, MaybePuzzle> {
    using type = __move_t<get_hole_position_t<MaybePuzzle>::value % 4 != 3, get_hole_position_t<MaybePuzzle>::value, Left , MaybePuzzle::value>;
    static constexpr Movement direction = Left;
};

/* Try to move nothing results nothing */
template<>
struct move<Up   , Nothing> { using type = Nothing; };
template<>
struct move<Right, Nothing> { using type = Nothing; };
template<>
struct move<Down , Nothing> { using type = Nothing; };
template<>
struct move<Left , Nothing> { using type = Nothing; };

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

#endif

