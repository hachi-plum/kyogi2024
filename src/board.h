#pragma once

#include <vector>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Board {
protected:
    std::vector<std::vector<int>> board;
    std::vector<std::vector<int>> board_goal;
    std::pair<int, int> board_size;
    std::vector<std::vector<std::vector<int>>> patterns;
    int count = 0;
    std::vector<std::tuple<int, int, int, int>> apply_mask_shifts_history;
    std::pair<int, int> matched_rowcol = {0, 0};
    std::map<int, int> size2pidx = {
        {1, 0},
        {2, 1},
        {4, 4},
        {8, 7},
        {16, 10},
        {32, 13},
        {64, 16},
        {128, 19},
        {256, 22}
    };
    int boardsize_pattern;

    std::vector<std::vector<std::vector<int>>> gen_patterns(const json &state);

public:
    Board(const json &state);
    Board(const json &state, bool flip);
    void apply_mask_shifts(const int &pidx, const std::pair<int, int> &mask_position, const int &dir);
    int count_matching_elements();
    int nextPowerOfTwo(int n);
    unsigned int prevPowerOf2(unsigned int n);
    bool isPowerOfTwo(int x);
    int find_best_matching_row(int row_number);
    std::pair<std::pair<int, int>, int> findLongestMatchingSequence(const std::pair<int, int> &destination);
    std::pair<int, int> find_addition_length_samerow(const std::pair<int, int> &destination, const int &start_row, const int &moved_length);
    int check_matched_row();
    int calculateTotalSquaredDifference();
    void start_process();
    void create_json_file(const std::string &filename) const;
    void printUnmatched();
    void print_board();
    void print_goal_board();
    void print_patterns();
    void apply_history_from_file(const std::string &filename);
    int get_count();
};