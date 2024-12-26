#include <vector>
#include <cstdlib>
#include <unordered_map>
#include <ctime>          // For srand(time(0))
#include "crow.h"     // Include Crow header

using namespace std;

class Sudoku {
private:
    int grid[9][9];      // Sudoku grid
    int solution[9][9];  // Solved Sudoku grid for validation

    // Check if placing a number in a cell is valid
    bool isValidMove(int row, int col, int num) {
        // Check row and column
        for (int i = 0; i < 9; i++) {
            if (grid[row][i] == num || grid[i][col] == num)
                return false;
        }

        // Check 3x3 sub-grid
        int startRow = row - row % 3, startCol = col - col % 3;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (grid[startRow + i][startCol + j] == num)
                    return false;
            }
        }

        return true;
    }

    // Solve Sudoku using backtracking
    bool solveSudoku() {
        for (int row = 0; row < 9; row++) {
            for (int col = 0; col < 9; col++) {
                if (grid[row][col] == 0) {
                    for (int num = 1; num <= 9; num++) {
                        if (isValidMove(row, col, num)) {
                            grid[row][col] = num;
                            if (solveSudoku())
                                return true;
                            grid[row][col] = 0; // Backtrack
                        }
                    }
                    return false;
                }
            }
        }
        return true;
    }

    // Copy the solution to a separate grid for validation
    void copySolution() {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                solution[i][j] = grid[i][j];
            }
        }
    }

public:
    // Constructor to initialize the grid
    Sudoku() {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                grid[i][j] = 0;
            }
        }
    }

    // Generate a valid Sudoku grid
    void generateGrid() {
        srand(time(0));
        for (int i = 0; i < 9; i++) {
            int row = rand() % 9;
            int col = rand() % 9;
            int num = rand() % 9 + 1;
            if (isValidMove(row, col, num)) {
                grid[row][col] = num;
            }
        }
        solveSudoku();
        copySolution();
    }

    // Remove numbers to create a puzzle
    void removeNumbers(int difficulty) {
        int attempts = difficulty * 10; // Increase for higher difficulty
        while (attempts > 0) {
            int row = rand() % 9;
            int col = rand() % 9;
            if (grid[row][col] != 0) {
                grid[row][col] = 0;
                attempts--;
            }
        }
    }

    // Get the current grid cell value
    int getCell(int row, int col) {
        return grid[row][col];
    }

    // Validate the user's move
    bool validateMove(int row, int col, int num) {
        return solution[row][col] == num;
    }
};

int main() {
    Sudoku game;
    game.generateGrid();
    game.removeNumbers(2);  // Difficulty level 2 (medium)

    crow::SimpleApp app;
    unordered_map<string, int> attempts; // Tracks attempts per cell

    // Endpoint to get the initial grid
    CROW_ROUTE(app, "/getGrid").methods("GET"_method)([&]() {
        crow::json::wvalue response;
        vector<vector<int>> grid(9, vector<int>(9));
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                grid[i][j] = game.getCell(i, j);
            }
        }
        response["grid"] = grid;
        return response;
    });

    // Endpoint to submit a move
    CROW_ROUTE(app, "/submitMove").methods("POST"_method)([&](const crow::request &req) {
        auto body = crow::json::load(req.body);
        if (!body)
            return crow::response(400);

        int row = body["row"].i();
        int col = body["col"].i();
        int num = body["num"].i();

        string key = to_string(row) + "," + to_string(col);

        if (attempts[key] >= 2) {
            return crow::response(403, "No attempts left for this cell. Game Over!");
        }

        if (game.validateMove(row, col, num)) {
            attempts.erase(key); // Reset attempts if correct
            return crow::response(200, "Correct move!");
        } else {
            attempts[key]++;
            if (attempts[key] >= 2) {
                return crow::response(403, "No attempts left for this cell. Game Over!");
            } else {
                return crow::response(200, "Wrong move! You have " + to_string(2 - attempts[key]) + " attempt(s) left.");
            }
        }
    });

    app.port(3000).multithreaded().run();
}
