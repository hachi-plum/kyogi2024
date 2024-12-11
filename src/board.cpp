#include <iostream>
#include <vector>
#include <algorithm>
#include <set>
#include <fstream>
#include <nlohmann/json.hpp>
#include <climits>
#include <map>
#include "board.h"

using json = nlohmann::json;

// 定型抜き型の生成
std::vector<std::vector<std::vector<int>>> Board::gen_patterns(const json &state)
{
    std::vector<int> sizes = {2, 4, 8, 16, 32, 64, 128, 256};
    std::vector<std::vector<std::vector<int>>> arrays;

    arrays.push_back({{1}});

    for (int size : sizes)
    {
        // すべての要素が1の配列
        arrays.push_back(std::vector<std::vector<int>>(size, std::vector<int>(size, 1)));

        // 奇数行が1の配列
        std::vector<std::vector<int>> odd_rows(size, std::vector<int>(size, 0));
        for (int i = 0; i < size; i += 2)
        {
            std::fill(odd_rows[i].begin(), odd_rows[i].end(), 1);
        }
        arrays.push_back(odd_rows);

        // 奇数列が1の配列
        std::vector<std::vector<int>> odd_cols(size, std::vector<int>(size, 0));
        for (int i = 0; i < size; ++i)
        {
            for (int j = 0; j < size; j += 2)
            {
                odd_cols[i][j] = 1;
            }
        }
        arrays.push_back(odd_cols);
    }
    // 一般抜き型追加
    for (const auto &pattern : state["general"]["patterns"])
    {
        std::vector<std::vector<int>> new_pattern;
        for (const auto &row : pattern["cells"])
        {
            std::vector<int> new_row;
            for (char c : row.get<std::string>())
            {
                new_row.push_back(c - '0');
            }
            new_pattern.push_back(new_row);
        }
        arrays.push_back(new_pattern);
    }

    return arrays;
}

Board::Board(const json &state)
{
    board_size = {state["board"]["height"], state["board"]["width"]};
    board.resize(board_size.first);
    for (int i = 0; i < board_size.first; ++i)
    {
        board[i].resize(board_size.second);
        std::string row = state["board"]["start"][i];
        if (row.length() != board_size.second)
        {
            throw std::runtime_error("Row length mismatch in start board");
        }
        for (int j = 0; j < board_size.second; ++j)
        {
            if (row[j] < '0' || row[j] > '9')
            {
                throw std::runtime_error("Invalid character in start board");
            }
            board[i][j] = row[j] - '0';
        }
    }

    board_goal.resize(board_size.first);
    for (int i = 0; i < board_size.first; ++i)
    {
        board_goal[i].resize(board_size.second);
        std::string row = state["board"]["goal"][i];
        for (int j = 0; j < board_size.second; ++j)
        {
            board_goal[i][j] = row[j] - '0';
        }
    }

    patterns = gen_patterns(state);
    boardsize_pattern = size2pidx[nextPowerOfTwo(board_size.second)];
}

Board::Board(const json &state, bool flip) {}

void Board::apply_mask_shifts(const int &pidx, const std::pair<int, int> &mask_position, const int &dir = 0)
{
    std::vector<std::vector<int>> mask = patterns[pidx];
    std::vector<std::vector<int>> board_mask(board_size.first, std::vector<int>(board_size.second, 0));
    if(!(mask.size()+mask_position.first <= 0 || mask_position.first >= board_size.first || mask[0].size()+mask_position.second <= 0 || mask_position.second >= board_size.second)){
        for (int y = 0; y < mask.size(); y++)
        {
            for (int x = 0; x < mask[y].size(); x++)
            {
                if (mask[y][x] == 1)
                {
                    int y2 = y + mask_position.first;
                    int x2 = x + mask_position.second;
                    if (y2 < 0 || y2 >= board_size.first || x2 < 0 || x2 >= board_size.second)
                    {
                        continue;
                    }
                    board_mask[y2][x2] = 1;
                }
            }
        }

        if (dir == 1)
        { // 上
            for (int i = std::max(mask_position.second, 0); i < std::min(mask_position.second + (int)mask[0].size(), board_size.second); i++)
            {
                int skip = 0;
                std::vector<int> mask_n;
                for (int j = board_size.first - 1; j >= 0; j--)
                {
                    while (j - skip >= 0 && board_mask[j - skip][i] == 1)
                    {
                        mask_n.push_back(board[j - skip][i]);
                        skip++;
                    }
                    if (j - skip < 0)
                    {
                        board[j][i] = mask_n[skip - j - 1];
                    }
                    else
                    {
                        board[j][i] = board[j - skip][i];
                    }
                }
            }
        }
        else if (dir == 0)
        { // 下
            for (int i = std::max(mask_position.second, 0); i < std::min(mask_position.second + (int)mask[0].size(), board_size.second); i++)
            {
                int skip = 0;
                std::vector<int> mask_n;
                for (int j = 0; j < board_size.first; j++)
                {
                    while (j + skip < board_size.first && board_mask[j + skip][i] == 1)
                    {
                        mask_n.push_back(board[j + skip][i]);
                        skip++;
                    }
                    if (j + skip >= board_size.first)
                    {
                        board[j][i] = mask_n[j + skip - board_size.first];
                    }
                    else
                    {
                        board[j][i] = board[j + skip][i];
                    }
                }
            }
        }
        else if (dir == 3)
        { // 左
            for (int i = std::max(mask_position.first, 0); i < std::min(mask_position.first + (int)mask.size(), board_size.first); i++)
            {
                int skip = 0;
                std::vector<int> mask_n;
                for (int j = board_size.second - 1; j >= 0; j--)
                {
                    while (j - skip >= 0 && board_mask[i][j - skip] == 1)
                    {
                        mask_n.push_back(board[i][j - skip]);
                        skip++;
                    }
                    if (j - skip < 0)
                    {
                        board[i][j] = mask_n[skip - j - 1];
                    }
                    else
                    {
                        board[i][j] = board[i][j - skip];
                    }
                }
            }
        }
        else if (dir == 2)
        { // 右
            for (int i = std::max(mask_position.first, 0); i < std::min(mask_position.first + (int)mask.size(), board_size.first); i++)
            {
                int skip = 0;
                std::vector<int> mask_n;
                for (int j = 0; j < board_size.second; j++)
                {
                    while (j + skip < board_size.second && board_mask[i][j + skip] == 1)
                    {
                        mask_n.push_back(board[i][j + skip]);
                        skip++;
                    }
                    if (j + skip >= board_size.second)
                    {
                        board[i][j] = mask_n[j + skip - board_size.second];
                    }
                    else
                    {
                        board[i][j] = board[i][j + skip];
                    }
                }
            }
        }

        apply_mask_shifts_history.push_back({pidx, mask_position.first, mask_position.second, dir});
        count++;
    }else{
        std::cout << "!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    }
}

int Board::count_matching_elements()
{
    int count = 0;
    for (int i = 0; i < board_size.first; ++i)
    {
        for (int j = 0; j < board_size.second; ++j)
        {
            if (board[i][j] == board_goal[i][j])
            {
                ++count;
            }
        }
    }
    return count;
}

// nより大きい最小の2のべき乗を返す
int Board::nextPowerOfTwo(int n)
{
    if (n <= 0)
        return 1;

    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;

    return n;
}

// n 以下の最大の 2 のべき乗を返す関数
unsigned int Board::prevPowerOf2(unsigned int n)
{
    // n が 0 の場合は 0 を返す
    if (n == 0)
        return 0;

    n |= (n >> 1);
    n |= (n >> 2);
    n |= (n >> 4);
    n |= (n >> 8);
    n |= (n >> 16);

    return (n + 1) >> 1;
}

// 2の累乗かどうかを確認する関数
bool Board::isPowerOfTwo(int x)
{
    return (x > 0) && ((x & (x - 1)) == 0);
}

// 最初に一番一致させやすい行を一番上に持ってくる用
int Board::find_best_matching_row(int row_number)
{
    std::vector<int> goal_counts(4, 0);
    for (int value : board_goal[row_number])
    {
        goal_counts[value]++;
    }

    int best_row = -1;
    int min_diff_sum = INT_MAX;

    for (int i = 0; i < board_size.first; i++)
    {
        std::vector<int> row_counts(4, 0);
        for (int value : board[i])
        {
            row_counts[value]++;
        }

        int diff_sum = 0;
        for (int j = 0; j < 4; j++)
        {
            int diff = row_counts[j] - goal_counts[j];
            diff_sum += diff * diff;
        }

        if (diff_sum < min_diff_sum)
        {
            min_diff_sum = diff_sum;
            best_row = i;
        }
    }
    return best_row;
}

std::pair<std::pair<int, int>, int> Board::findLongestMatchingSequence(const std::pair<int, int> &destination)
{
    int max_length = 0, length = 0, i, j;
    std::pair<int, int> best_start = {-1, -1};

    // ゴールボードの指定された位置から始まるシーケンスを取得
    std::vector<int> goal_sequence;
    for (j = destination.second; j < board_size.second; ++j)
    {
        goal_sequence.push_back(board_goal[destination.first][j]);
    }

    // 現在のボード上のすべての位置で検索
    for (j = 0; j < board_size.second - matched_rowcol.second; ++j)
    {
        length = 0;
        while (j + length < board_size.second - matched_rowcol.second && length < goal_sequence.size() && board[0][j + length] == goal_sequence[length])
        {
            length++;
        }

        if (length > max_length)
        {
            max_length = length;
            best_start = {0, j};
        }
    }
    for (i = 1; i < board_size.first - matched_rowcol.first; ++i)
    {
        for (j = 0; j < board_size.second; ++j)
        {
            length = 0;
            while (j + length < board_size.second && i + length < board_size.first - matched_rowcol.first && length < goal_sequence.size() && board[i][j + length] == goal_sequence[length])
            {
                length++;
            }

            if (length > max_length)
            {
                max_length = length;
                best_start = {i, j};
            }
        }
    }
    return {best_start, max_length};
}

std::pair<int, int> Board::find_addition_length_samerow(const std::pair<int, int> &destination, const int &start_row, const int &moved_length)
{
    int max_addition_length=0, addition_length=0, i, best_col = -1;
    std::vector<int> addition_sequence;
    for (i = destination.second; i<board_size.second; i++)
    {
        addition_sequence.push_back(board_goal[destination.first][i]);
    }
    for(i=0;i<board_size.second-moved_length;i++)
    {
        addition_length = 0;
        while(i+addition_length<board_size.second-moved_length && addition_length<addition_sequence.size() && nextPowerOfTwo(moved_length+addition_length+1) < board_size.first-matched_rowcol.first-start_row && board[start_row][i+addition_length] == addition_sequence[addition_length])
        {
            addition_length++;
        }
        if(prevPowerOf2(addition_length) > max_addition_length)
        {
            max_addition_length = prevPowerOf2(addition_length);
            best_col = i;
        }
    }
    return {best_col, max_addition_length};
}

int Board::check_matched_row()
{
    int i, match_row = 0;

    for (i = 0; i < board_size.first - matched_rowcol.first; i++)
    {
        // vector<int>のデータ部分をポインタで取得し、memcmpで比較
        if (memcmp(board[i].data(), board_goal[i + matched_rowcol.first].data(), sizeof(int) * board[i].size()) != 0)
        {
            break; // 一致しなければ終了
        }
        match_row++;
    }

    if (match_row > 0)
    {
        apply_mask_shifts(boardsize_pattern, {match_row - nextPowerOfTwo(board_size.second), 0}, 0);
        matched_rowcol.first += match_row;
    }
    //std::cout << "mathced_rowcol.first: " << matched_rowcol.first << std::endl;
    //std::cout << "matched_row: " << match_row << std::endl;
    return match_row;
}

int Board::calculateTotalSquaredDifference()
{
    int total_diff_sum = 0;
    int board_counts[4] = {0}; // 固定サイズの配列を使用
    int goal_counts[4] = {0};

    for (int row = 0; row < board_size.first; row++)
    {
        // 配列をリセット
        std::fill(board_counts, board_counts + 4, 0);
        std::fill(goal_counts, goal_counts + 4, 0);

        // 1つのループで board と board_goal のカウントを同時に行う
        for (int col = 0; col < board_size.second; col++)
        {
            board_counts[board[row][col]]++;
            goal_counts[board_goal[row][col]]++;
        }

        // 差分の計算
        for (int i = 0; i < 4; i++)
        {
            int diff = board_counts[i] - goal_counts[i];
            total_diff_sum += diff * diff;
        }
    }

    return total_diff_sum;
}

void Board::start_process()
{
    int i, j, k, l, match_row = 0, additional_row, next_length, prev_length, add_length = 0;
    bool match = true;
    std::pair<std::pair<int, int>, int> result;
    std::pair<int, int> additional_col;

    matched_rowcol.first = check_matched_row();

    for (i = matched_rowcol.first; i < board_size.first; i += match_row)
    {
        matched_rowcol.second = 0;
        for (j = 0; j < board_size.second; j += add_length)
        {
            result = findLongestMatchingSequence({i, j});
            //std::cout << "searching: " << i << " " << j << std::endl;
            //std::cout << "result: " << result.first.first << " " << result.first.second << " length:" << result.second << std::endl;
            if(result.first.first == 0 && result.first.second == board_size.second - result.second)
            {
                add_length = result.second;
            }
            if (result.first.first == 0)
            {
                additional_row = 1;
                for (k = 1; k < result.second && i+k < board_size.first; k++)
                {
                    for (l = 0; l < result.second && j+l < board_size.second && match; l++)
                    {
                        if (board[result.first.first + k][result.first.second + l] != board_goal[i + k][j + l])
                        {
                            match = false;
                        }
                    }
                    if (!match)
                    {
                        break;
                    }
                    additional_row++;
                }
                next_length = nextPowerOfTwo(result.second);
                if (result.first.second == 0)
                {
                    apply_mask_shifts(size2pidx[next_length], {additional_row - next_length, result.second - next_length}, 2);
                    add_length = result.second;
                }
                else if (!isPowerOfTwo(result.second) && board_size.second - matched_rowcol.second - nextPowerOfTwo(result.second) - result.first.second >= 0)
                {
                    apply_mask_shifts(size2pidx[next_length], {additional_row - next_length, result.first.second}, 3);
                    apply_mask_shifts(size2pidx[next_length], {additional_row - next_length, result.second - next_length}, 2);
                    add_length = result.second;
                }
                else
                {
                    prev_length = prevPowerOf2(result.second);
                    apply_mask_shifts(size2pidx[prev_length], {additional_row - prev_length, result.first.second}, 2);
                    add_length = prev_length;
                }

            }
            else
            {
                if(result.first.second == 0)
                {
                    if (board_size.first - matched_rowcol.first - result.first.first - nextPowerOfTwo(result.second) >= 0)
                    {
                        next_length = nextPowerOfTwo(result.second);
                        apply_mask_shifts(size2pidx[next_length], {result.first.first, result.second - next_length}, 1);
                        apply_mask_shifts(size2pidx[next_length], {0, result.second - next_length}, 2);
                        add_length = result.second;
                    }
                    else
                    {
                        prev_length = prevPowerOf2(result.second);
                        apply_mask_shifts(size2pidx[prev_length], {result.first.first, 0}, 1);
                        apply_mask_shifts(size2pidx[prev_length], {0, 0}, 2);
                        add_length = prev_length;
                    }
                }else if(result.first.second == board_size.second - result.second && board_size.first - matched_rowcol.first - result.first.first - nextPowerOfTwo(result.second) >= 0){//すでに見つけた場所が右端にある場合
                    next_length = nextPowerOfTwo(result.second);
                    add_length = result.second;
                    additional_col = find_addition_length_samerow({i, j+result.second}, result.first.first, result.second);
                    if(additional_col.first != -1)
                    {
                        apply_mask_shifts(size2pidx[additional_col.second], {result.first.first, additional_col.first}, 2);
                        add_length += additional_col.second;
                        next_length = nextPowerOfTwo(add_length);
                    }
                    apply_mask_shifts(size2pidx[next_length], {1-next_length, add_length-next_length}, 2);
                    apply_mask_shifts(size2pidx[next_length], {result.first.first, board_size.second-add_length}, 1);
                }else if(result.first.second+prevPowerOf2(result.second) <= board_size.second - matched_rowcol.second){
                    prev_length = prevPowerOf2(result.second);
                    apply_mask_shifts(size2pidx[prev_length], result.first, 1);
                    apply_mask_shifts(size2pidx[prev_length], {0, result.first.second}, 2);
                    add_length = prev_length;
                }else if(result.first.first >= nextPowerOfTwo(result.second)){//右側に移動できる場合
                    if(result.first.first >= nextPowerOfTwo(result.second) && board_size.first - matched_rowcol.first - result.first.first - nextPowerOfTwo(result.second) >= 0)
                    {
                        next_length = nextPowerOfTwo(result.second);
                        apply_mask_shifts(size2pidx[next_length], {result.first.first, result.first.second+result.second-next_length}, 2);
                        add_length = result.second;
                        additional_col = find_addition_length_samerow({i, j+result.second}, result.first.first, result.second);
                        if(additional_col.first != -1)
                        {
                            apply_mask_shifts(size2pidx[additional_col.second], {result.first.first, additional_col.first}, 2);
                            add_length += additional_col.second;
                            next_length = nextPowerOfTwo(add_length);
                        }
                        apply_mask_shifts(size2pidx[next_length], {1-next_length, add_length-next_length}, 2);
                        apply_mask_shifts(size2pidx[next_length], {result.first.first, board_size.second-add_length}, 1);
                    }else{
                        prev_length = prevPowerOf2(result.second);
                        apply_mask_shifts(size2pidx[prev_length], {result.first.first, result.first.second}, 2);
                        apply_mask_shifts(size2pidx[prev_length], {0, 0}, 2);
                        apply_mask_shifts(size2pidx[prev_length], {result.first.first, board_size.second-prev_length}, 1);
                        add_length = prev_length;
                    }
                }else{
                    next_length = nextPowerOfTwo(result.second);
                    if (board_size.first - matched_rowcol.first - result.first.first - next_length >= 0)
                    {
                        apply_mask_shifts(size2pidx[next_length], {result.first.first, result.first.second}, 3);
                        apply_mask_shifts(size2pidx[next_length], {result.first.first, result.second - next_length}, 1);
                        apply_mask_shifts(size2pidx[next_length], {0, result.second - next_length}, 2);
                        add_length = result.second;
                    }
                    else
                    {
                        prev_length = prevPowerOf2(result.second);
                        apply_mask_shifts(size2pidx[prev_length], {result.first.first, result.first.second}, 3);
                        apply_mask_shifts(size2pidx[prev_length], {result.first.first, 0}, 1);
                        apply_mask_shifts(size2pidx[prev_length], {0, 0}, 2);
                        add_length = prev_length;
                    }
                }
            }
            matched_rowcol.second += add_length;
        }
        match_row = check_matched_row();
    }
}

void Board::create_json_file(const std::string &filename) const
{
    json output;
    output["n"] = count;

    json ops = json::array();
    for (const auto &[pidx, y, x, dir] : apply_mask_shifts_history)
    {
        ops.push_back({{"p", pidx},
                       {"x", x},
                       {"y", y},
                       {"s", dir}});
    }
    output["ops"] = ops;

    std::ofstream file(filename);
    if (file.is_open())
    {
        file << output.dump(2); // Use 2 spaces for indentation
        file.close();
        std::cout << "JSON file created successfully: " << filename << std::endl;
    }
    else
    {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }
}

// 一致していないマスの座標を表示
void Board::printUnmatched()
{
    for (int i = 0; i < board_size.first; ++i)
    {
        for (int j = 0; j < board_size.second; ++j)
        {
            if (board[i][j] != board_goal[i][j])
            {
                std::cout << "(" << i << ", " << j << ") ";
            }
        }
    }
    std::cout << std::endl;
}

void Board::print_board()
{
    for (const auto &row : board)
    {
        for (int elem : row)
        {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void Board::print_goal_board()
{
    for (const auto &row : board_goal)
    {
        for (int elem : row)
        {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void Board::print_patterns()
{
    for (int i = 0; i < patterns.size(); ++i)
    {
        std::cout << "Pattern " << i << ":" << std::endl;
        for (const auto &row : patterns[i])
        {
            for (int elem : row)
            {
                std::cout << elem << " ";
            }
            std::cout << std::endl;
        }
    }
}

// ファイルから操作履歴を読み込む
void Board::apply_history_from_file(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Unable to open file: " + filename);
    }

    json answer_json;
    file >> answer_json;

    if (!answer_json.contains("ops"))
    {
        throw std::runtime_error("Invalid JSON format: 'ops' key not found");
    }

    for (const auto &op : answer_json["ops"])
    {
        int pidx = op["p"];
        int y = op["y"];
        int x = op["x"];
        int dir = op["s"];

        apply_mask_shifts(pidx, {y, x}, dir);
    }

    std::cout << "Applied " << answer_json["ops"].size() << " operations from " << filename << std::endl;
}

int Board::get_count() {
    return count;
}