#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "board.h"

using json = nlohmann::json;

int main(int argc, char* argv[]) {
    if(argc < 2 || argc > 4) {
        std::cerr << "Usage: " << argv[0] << " <input_file.json> [-h <history_file.json>]" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream file(filename);
    json state;
    file >> state;

    Board board(state);
    std::cout << "Initial Board:" << std::endl;
    board.print_board();
    std::cout << "Goal Board:" << std::endl;
    board.print_goal_board();

    std::pair<int, int> board_size = {state["board"]["height"], state["board"]["width"]};

    bool apply_history = false;
    std::string history_filename = "h_answer.json";  // Default history filename

    if (argc >= 3 && std::string(argv[2]) == "-h") {
        apply_history = true;
        if (argc == 4) {
            history_filename = argv[3];
        }
    }
    if (apply_history) {
        try {
            board.apply_history_from_file(history_filename);
            board.print_board();
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }
    board.start_process();

    if(board.count_matching_elements() == board_size.first * board_size.second) {
        std::cout << "Goal board reached!" << std::endl;
    } else {
        std::cout << "Goal board not reached." << std::endl;
        board.printUnmatched();
    }
    board.create_json_file("answer.json");
    return 0;
}