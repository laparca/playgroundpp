#include <memory>
#include <vector>
#include <algorithm>
#include <utility>
#include <queue>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <chrono>

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
    for (int p = 0; p < 16; p++) {
        result.push_back({
            v & 0xfull,
            p
        });
        v >>= 4;
    }
    return result;
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

int manhattam_distances_sum_of(Board b) {
    int sum = 0;
    auto positions = get_positions(b);
    std::vector<int> manhattan_distances;

    std::transform(positions.begin(), positions.end(), std::back_inserter(manhattan_distances), [](std::pair<int, int>& p) {
        int value = p.first;
        int position = p.second;

        coord c_orig = position_to_coord(position);
        coord c_dest = position_to_coord(15 - ((value + 15) % 16));
        
        return manhattan_distance(c_orig, c_dest);
    });

    std::for_each(manhattan_distances.begin(), manhattan_distances.end(), [&sum](int v) {
        sum += v;
    });

    return sum;
}

struct Status {
    Board board;
    int moves;
    int md;
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

    status_path() = default;
    status_path(const status_path&) = default;
    status_path(status_path&&) = default;

    status_path& operator=(const status_path&) = default;
    status_path& operator=(status_path&&) = default;

    bool has_better_heuristic_than(const status_path& sp) const {
        int a_sum = path.back().moves + path.back().md;
        int b_sum = sp.path.back().moves + sp.path.back().md;
        return (a_sum > b_sum) || (a_sum == b_sum && path.back().moves > sp.path.back().moves);
    }

    int moves() const {
        return path.size() - 1;
    }

    status_path operator+(const Status& s) const {
        status_path sp{*this};
        sp.path.push_back(s);
        return sp;
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
};

struct HeuristicCompareStatus {
    bool operator()(const status_path& a, const status_path& b) const {
        return a.has_better_heuristic_than(b);
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

using priority_queue = std::priority_queue<status_path, std::vector<status_path>, HeuristicCompareStatus>;

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
        s << q.top();
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



int main(int argc, const char** argv) {
    priority_queue queue;
    std::vector<Status> visited;
    constexpr Direction directions[] = { Up, Right, Down, Left };
    uint64_t count = 0;
    uint64_t max = 100;
    int percent = 0;
    uint64_t old_count = 0;
    int visited_inserts = 0;
    int visited_replaced = 0;

    if (argc > 1)
        max = std::stoull(argv[1]);
    
    queue.push(status_path{Status(0x01234'5678'9abc'def)});
    auto start_time = std::chrono::system_clock::now();
    while(!queue.top().is_solved() && !queue.empty()) {
//        std::cout << queue << std::endl;

        auto top = queue.top();
        queue.pop();

        auto it = std::find_if(visited.begin(), visited.end(), [&top](auto const& visited){
            return top.path.back().board == visited.board;
        });

        if (it == visited.end()) {
            // If not found, insert it
            visited.push_back(top.last());
            visited_inserts ++;
        }
        else if (it->moves > top.last().moves) {
            // The new node is equal, but with a smaller path
            *it = top.last();
            visited_replaced ++;
        }

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

        // Remove visied moves if its path is longer than the stored.
        new_status.erase(std::remove_if(new_status.begin(), new_status.end(), [&visited](const Status& s) {
            return visited.end() != std::find_if(visited.begin(), visited.end(), [s](const Status& v){
                return s.board == v.board && s.moves > v.moves;
            });
        }), new_status.end());

        std::for_each(new_status.begin(), new_status.end(), [&queue, &top](auto& s) {
            queue.push(top + s);
        });

        if (++count == max)
            break;

        int new_percent = (count * 100) / max;
        if (new_percent != percent) {
            auto t = std::chrono::system_clock::now();
            std::chrono::duration<double> diff = t - start_time;
            std::cout << "count = " << count 
                      << "; duration = " << diff.count()
                      << "; percent = " << new_percent << " %"
                      << "; visited_inserts = " << visited_inserts
                      << "; visited_replaced = " << visited_replaced
                      << "; " << std::setprecision(10) << (count - old_count) / diff.count() << " counts / s"
                      << std::endl;
            start_time = t;
            old_count = count;
            percent = new_percent;
            visited_inserts = 0;
            visited_replaced = 0;
        }

    }

    std::cout << "Iterations: " << count << std::endl;
    std::cout << "Queue size: " << queue.size() << std::endl;
    std::cout << "Visited   : " << visited.size() << std::endl;

    if (queue.empty() || !queue.top().is_solved()) {
        std::cout << "There isn't solution" << std::endl;
        std::cout << "Best found until now is " << queue.top() << std::endl;
    }
    else {
        std::cout << "Solution: " << queue.top() << std::endl;
    }

    return 0;
}
