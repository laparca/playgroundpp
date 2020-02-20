#include <memory>
#include <vector>
#include <deque>
#include <algorithm>
#include <utility>
#include <queue>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <chrono>
#include <signal.h>
#include <string_view>
#include <playgroundcpp/steps.hh>

struct coord {
    int x;
    int y;
};

coord operator+(const coord& a, const coord& b) {
    return { a.x + b.x, a.y + b.y };
}

int manhattan_distance(const coord& a, const coord& b) {
    return std::pow(std::abs(a.x - b.x) + std::abs(a.y - b.y), 2);
}

enum Direction {
    Up,
    Right,
    Down,
    Left,
    Nothing
};

constexpr coord direction_to_coord[]{
    /*[Up   ] =*/ { 0,  1},
    /*[Right] =*/ {-1,  0},
    /*[Down ] =*/ { 0, -1},
    /*[Left ] =*/ { 1,  0}
};

coord position_to_coord(int p) {
    return {p % 4, p / 4};
}

int coord_to_position(const coord& c) {
    return (c.y * 4) + c.x;
}

using Board = uint64_t;

int get_hole_position(const Board& b) {
    int i = 0;
    for (auto a = b; a & 0xfull; a >>= 4)
        i++;
    return i;
}

std::vector<std::pair<int, int>> get_positions(const Board& b) {
    std::vector<std::pair<int, int>> result;
    uint64_t v = b;

    using namespace playgroundcpp;

    16_times([&v, &result](std::size_t p){
        result.push_back({
            v & 0xfull,
            p
        });
        v >>= 4;
    });

    return result;
}

int count_board_inversions(const Board& b) {
    auto p = get_positions(b);
    int inversions = 0;

    for (std::size_t i = 0; i < p.size() - 1; i++)
        for (std::size_t j = i+1; j < p.size(); j++)
            if (p[i].first != 0 && p[j].first != 0 && p[i].first > p[j].first)
                inversions++;

    return inversions;
}

bool is_board_solvable(const Board& b) {
    int count = count_board_inversions(b);

    return (count % 2) == 0;
}

std::optional<Board> move(const Board& b, Direction d) {
    int hole_pos = get_hole_position(b);
    coord hole = position_to_coord(hole_pos);
    coord dest = hole + direction_to_coord[d];
    if (dest.x < 0 || dest.x > 3 || dest.y < 0 || dest.y > 3)
       return {};

    int dest_pos = coord_to_position(dest);
    // v is the value to move to the old hole position
    uint64_t v = (b & (0xfull << (4 * dest_pos)));

    if (hole_pos > dest_pos)
        v <<= (4*(hole_pos - dest_pos));
    else
        v >>= (4*(dest_pos - hole_pos));

    Board new_board = (b | v) & ~(0xfull << (4 * dest_pos));

    return {new_board};
}

uint64_t manhattam_distances_sum_of(Board b) {
    uint64_t sum = 0;
    auto positions = get_positions(b);
    std::vector<uint64_t> manhattan_distances;

    std::transform(positions.begin(), positions.end(), std::back_inserter(manhattan_distances), [](std::pair<int, int>& p) {
        int value = p.first;
        int position = p.second;
        int corrected_value = 15 - ((value + 15) % 16);

        coord c_orig = position_to_coord(position);
        coord c_dest = position_to_coord(corrected_value);
        
        return manhattan_distance(c_orig, c_dest);
        //return manhattan_distance(c_orig, c_dest) * (corrected_value / 4 == 3 ? 100 : corrected_value % 4 == 3 ? 50 : 1);
        //return std::pow(manhattan_distance(c_orig, c_dest), corrected_value + 1);
    });

    std::for_each(manhattan_distances.begin(), manhattan_distances.end(), [&sum](int v) {
        sum += v;
    });

    return sum;
}

struct Status {
    Board board;
    int moves;
    uint64_t md;
    Direction direction;

    Status()
        : board{0}
        , moves{0}
        , md{0}
        , direction{Nothing}
    {}

    Status(const Status&) = default;
    Status(Status&&) = default;

    explicit Status(Board board)
        : board{board}
        , moves{0}
        , md{manhattam_distances_sum_of(board)}
        , direction{Nothing}
    {}

    Status(Board board, int moves, Direction direction)
        : board{board}
        , moves{moves}
        , md{manhattam_distances_sum_of(board)}
        , direction{direction}
    {}

    Status& operator=(const Status&) = default;
    Status& operator=(Status&&) = default;
    
    bool is_solved() const {
        return board == 0x1234'5678'9abc'def0;
    }
};

struct status_path {
    std::vector<Status> path;

    explicit status_path(const Status& status) : path{status} {}
    explicit status_path(std::vector<Status>&& v) : path{std::move(v)} {}

    status_path() = default;
    status_path(const status_path&) = default;
    status_path(status_path&&) = default;

    status_path& operator=(const status_path&) = default;
    status_path& operator=(status_path&&) = default;

    bool has_better_heuristic_than(const status_path& sp) const {
        uint64_t a_sum =    last().moves +    last().md;
        uint64_t b_sum = sp.last().moves + sp.last().md;

        return a_sum < b_sum;
    }

    int moves() const {
        return path.size() - 1;
    }

    status_path operator+(const Status& s) const {
        std::vector<Status> new_vec(path.size() + 1);
        std::copy(path.begin(), path.end(), new_vec.begin());
        new_vec[path.size()] = s;
        return status_path(std::move(new_vec));
    }

    const Status& last() const {
        return path.back();
    }

    Status& last() {
        return path.back();
    }

    bool is_solved() const {
        return path.back().is_solved();
    }

    bool has_worse_than(const Status& status) {
        auto it = std::find_if(path.begin(), path.end(), [&status](auto const& s) {
            return s.board == status.board;
        });
        return it != path.end() && it->moves > status.moves;
    }
};

struct HeuristicCompareStatus {
    bool operator()(const status_path& a, const status_path& b) const {
        return !a.has_better_heuristic_than(b);
    }
};

template<class Stream>
Stream& operator<<(Stream& s, Direction d) {
    switch(d) {
        case Up     : s << "Up     "; break;
        case Right  : s << "Right  "; break;
        case Down   : s << "Down   "; break;
        case Left   : s << "Left   "; break;
        case Nothing: s << "Nothing"; break;
    }
    return s;
}

template<class Stream>
Stream& operator<<(Stream& s, const std::optional<Status>& status);

template<class Stream>
Stream& operator<<(Stream& s, const Status& status) {
    
    s << "{ board = " << std::hex << std::setw(16) << std::setfill('0') << status.board
      << ", moves = " << std::dec << status.moves 
      << ", md = " << std::dec << status.md 
      << ", direction = " << status.direction 
      << " }";
    return s;
}

template<class Stream>
Stream& operator<<(Stream& s, const status_path& status)
{
    bool has_comma = false;
    s << "[ ";
    for(auto& v : status.path) {
        if (has_comma)
            s << ", ";
        else
            has_comma = true;
        s << v;
    }
    s << " ]";

    return s;
}

using priority_queue = std::priority_queue<status_path, std::deque<status_path>, HeuristicCompareStatus>;

template<class Stream>
Stream& operator<<(Stream& s, priority_queue& q) {
    priority_queue tmp;
    bool show_comma = false;

    s << "[ ";
    while(!q.empty()) {
        if (show_comma)
            s << ", ";
        else
            show_comma = true;
        s << q.top().last();
        tmp.push(q.top());
        q.pop();
    }
    s << " ]";

    while(!tmp.empty()) {
        q.push(tmp.top());
        tmp.pop();
    }

    return s;
}

// From https://en.cppreference.com/w/cpp/algorithm/lower_bound
template<class Iterator, class T, class Func>
Iterator bin_search(Iterator first, Iterator last, const T& v, Func cmp) {
    first = std::lower_bound(first, last, v, cmp);
    return first != last && !cmp(v, *first) ? first : last;
}

#if (defined CLEAN_MEMORY) && !(defined CLEAM_MEMORY_LOOPS)
#define CLEAN_MEMORY_LOOPS 1000000
#endif

volatile bool sig_usr1 = false;

void sig_usr1_received(int) {
    sig_usr1 = true;
}

int main(int argc, const char** argv) {
#if defined USE_VISITED || defined CLEAN_MEMORY
    std::vector<Status> visited;
#endif
#if defined CLEAN_MEMORY
    size_t clean_counter = 0;
    priority_queue queues[2];
    int active_queue = 0;
    int alternative_queue = 1;
    int removed_paths = 0;
    std::chrono::duration<double> diff_clean;
#define queue queues[active_queue]
#define alter_queue queues[alternative_queue]
#else
    priority_queue queue;
#endif
    constexpr Direction directions[] = { Up, Right, Down, Left };
    uint64_t count = 0;
    uint64_t max = 100;
    int percent = 0;
    uint64_t old_count = 0;
    int visited_inserts = 0;
    int visited_replaced = 0;
    struct sigaction action;
    Board initial_board = 0x0123'4567'89ab'cdef;

    action.sa_handler = sig_usr1_received;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);

    sigaction(SIGUSR1, &action, nullptr);
    
    using namespace std::literals;
    
    for (int arg = 1; arg < argc; arg++) {
        if ("--board"sv == argv[arg]) {
            if ((arg + 1) < argc) {
                initial_board = std::stoull(argv[arg]);
            }
            else {
                std::cerr << "No board found" << std::endl;
                return 1;
            }
        }
        else
            max = std::stoull(argv[1]);
    }

    if (!is_board_solvable(initial_board)) {
        std::cerr << "The board cannot be solved." << std::endl;
        return 2;
    }

    queue.push(status_path{Status(initial_board)});
    auto start_time = std::chrono::system_clock::now();
    while(!queue.top().is_solved() && !queue.empty()) {
//        std::cout << queue << std::endl;

        auto top = queue.top();
        queue.pop();
        
        if (sig_usr1) {
            sig_usr1 = false;
            std::cerr << top << std::endl;
        }

#if defined USE_VISITED || defined CLEAN_MEMORY
#if defined USE_VISITED && defined SORTED_VISITED
        auto it = bin_search(visited.begin(), visited.end(), top.last(), [](auto const& a, auto const& b) { return a.board < b.board; });
#else
        auto it = std::find_if(visited.begin(), visited.end(), [&top](auto const& visited){
            return top.path.back().board == visited.board;
        });
#endif
        if (it == visited.end()) {
            // If not found, insert it
            visited.push_back(top.last());
#if defined USE_VISITED && defined SORTED_VISITED
            std::sort(visited.begin(), visited.end(), [](auto const& a, auto const& b) { return a.board < b.board; });
#endif
            visited_inserts ++;
        }
        else if (it->moves > top.last().moves) {
            // The new node is equal, but with a smaller path
            *it = top.last();
            visited_replaced ++;
        }
#endif
        std::vector<std::pair<std::optional<Board>, Direction>> boards(4);
        std::transform(std::begin(directions), std::end(directions), boards.begin(), [&top](Direction d) {
            return std::make_pair(move(top.last().board, d), d);
        });

        // Remove impossible moves
        boards.erase(std::remove_if(boards.begin(), boards.end(), [](auto& b) { return !b.first.has_value(); }), boards.end());

        std::vector<Status> new_status;
        std::transform(boards.begin(), boards.end(), std::back_inserter(new_status), [&top](auto& b) {
            return Status{*b.first, top.moves() + 1, b.second};
        });

#ifdef USE_VISITED
        // Remove visied moves if its path is longer than the stored.
        new_status.erase(std::remove_if(new_status.begin(), new_status.end(), [&visited](const Status& s) {
#ifdef SORTED_VISITED
            auto it = bin_search(visited.begin(), visited.end(), s, [](auto const& a, auto const& b){ return a.board < b.board; });
#else
            auto it = std::find_if(visited.begin(), visited.end(), [s](const Status& v){
                return s.board == v.board;
            });
#endif
            return it != visited.end() && s.moves > it->moves;
        }), new_status.end());
#endif

        auto& q = queue;
        std::for_each(new_status.begin(), new_status.end(), [&q, &top](auto& s) {
            if (top.path.end() == std::find_if(top.path.begin(), top.path.end(), [&s](auto const& s2) { return s.board == s2.board; })) {
                q.push(top + s);
            }
        });

        if (++count == max)
            break;

#ifdef CLEAN_MEMORY
        decltype(std::chrono::system_clock::now()) start_clean_time;
        decltype(std::chrono::system_clock::now()) stop_clean_time;
        if (++clean_counter >= CLEAN_MEMORY_LOOPS) {
            start_clean_time = std::chrono::system_clock::now();
            constexpr auto cmp = [](auto const& a, auto const& b) {
                return a.board < b.board;
            };
            std::sort(visited.begin(), visited.end(), cmp);
            while (!queue.empty()) {
                auto path = queue.top();
                bool insert = true;

                for(auto status : path.path) {
                    auto it = bin_search(visited.begin(), visited.end(), status, cmp);
                    if (it != visited.end() && status.moves > it->moves) {
                        insert = false;
                        break;
                    }
                }

                if (insert)
                    alter_queue.push(path);
                else
                    removed_paths ++;
                
                queue.pop();
            }
            active_queue = (active_queue + 1) % 2;
            alternative_queue = (active_queue + 1) % 2;
            clean_counter = 0;
            stop_clean_time = std::chrono::system_clock::now();
            diff_clean += stop_clean_time - start_clean_time;
        } 
#endif


        int new_percent = (count * 100) / max;
        if (new_percent != percent) {
            auto t = std::chrono::system_clock::now();
            std::chrono::duration<double> diff = t - start_time;
            std::cout << "count = " << count 
                      << "; duration = " << diff.count()
                      << "; percent = " << new_percent << " %"
                      << "; visited_inserts = " << visited_inserts
                      << "; visited_replaced = " << visited_replaced
#ifdef CLEAN_MEMORY
                      << "; clean duration = " << diff_clean.count()
                      << "; removed paths = " << removed_paths
#endif
                      << "; " << std::setprecision(10) << (count - old_count) / diff.count() << " counts / s"
                      << std::endl;
            start_time = t;
            old_count = count;
            percent = new_percent;
            visited_inserts = 0;
            visited_replaced = 0;
#ifdef CLEAN_MEMORY
            removed_paths = 0;
            diff_clean = decltype(diff_clean){};
#endif
        }

    }

    std::cout << "Iterations: " << count << std::endl;
    std::cout << "Queue size: " << queue.size() << std::endl;
#ifdef USE_VISITED
    std::cout << "Visited   : " << visited.size() << std::endl;
#endif

    if (queue.empty() || !queue.top().is_solved()) {
        std::cout << "There isn't solution" << std::endl;
        std::cout << "Best found until now is " << queue.top() << std::endl;
    }
    else {
        std::cout << "Solution: " << queue.top() << std::endl;
    }

    return 0;
}
