#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <string>

const int WindowWidth = 800;
const int WindowHeight = 450;
const float playerHeight = 1.0f;
const int worldSize = 120;

std::vector<std::vector<int>> generateWorld(int rows, int cols);

std::vector<std::vector<int>> simulateWorld(std::vector<std::vector<int>>* world, int worldSize);

int simulateCell(int row, int col, std::vector<std::vector<int>>* world, int maxRows, int maxCols);

int countNeighbors(std::vector<std::vector<int>>* world, int row, int col, int worldSize);

int main(void) {
    InitWindow(WindowWidth, WindowHeight, "Conway's Game of Life");

    Camera2D camera = { 0 };
    camera.target = (Vector2){ 0.0f, 0.0f };
    camera.offset = (Vector2){ WindowWidth / 2, WindowHeight / 2 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    std::vector<std::vector<int>> world = generateWorld(worldSize, worldSize);

    int generation = 0;

    bool running = false;

    float lastTime = 0.0f;
    float interval = 0.1f; // 1 second = 1.0f

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        if (running) {
            float currentTime = GetTime();
            if (currentTime - lastTime >= interval) {
                world = simulateWorld(&world, worldSize);
                lastTime = currentTime;
                generation++;
            }
        }

        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            float scaleFactor = 1.0f + (0.25f*fabsf(wheel));
            if (wheel < 0) scaleFactor = 1.0f/scaleFactor;
            camera.zoom = Clamp(camera.zoom*scaleFactor, 0.125f, 64.0f);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePosition = GetMousePosition();
            Vector2 worldPosition = GetScreenToWorld2D(mousePosition, camera);

            int col = (int)(worldPosition.x / 10);
            int row = (int)(worldPosition.y / 10);

            if (world[row][col] == 1) {
                world[row][col] = 0;
            } else {
                world[row][col] = 1;
            }
        }

        if (IsKeyDown(KEY_W)) {
            camera.target.y -= 5;
        }

        if (IsKeyDown(KEY_S)) {
            camera.target.y += 5;
        }

        if (IsKeyDown(KEY_A)) {
            camera.target.x -= 5;
        }

        if (IsKeyDown(KEY_D)) {
            camera.target.x += 5;
        }

        if (IsKeyPressed(KEY_SPACE)) {
            running = !running;
        }

        BeginDrawing();
            ClearBackground(BLACK);

            BeginMode2D(camera);

                for (int i = 0; i < worldSize; i++) {
                    for (int j = 0; j < worldSize; j++) {
                        if (world[i][j] == 1) {
                            // Render cells
                            DrawRectangle(j * 10, i * 10, 10, 10, GREEN);
                        }

                    }
                }

                for (int i = 0; i < worldSize; i++) {
                    for (int j = 0; j < worldSize; j++) {
                        if (i == 0) {
                            DrawLine(j * 10, 0, j * 10, worldSize * 10, DARKGRAY);
                        } else {
                            continue;
                        }
                    }
                    DrawLine(0, i * 10, worldSize * 10, i * 10, DARKGRAY);
                }

                // Additional grid lines to fix
                DrawLine(0, worldSize * 10, worldSize * 10, worldSize * 10, DARKGRAY);
                DrawLine(worldSize * 10, 0, worldSize * 10, worldSize * 10, DARKGRAY);

            EndMode2D();

            std::stringstream ss;
            ss << "Generation: " << generation;
            std::string result = ss.str();

            if (!running) {
                DrawText("PAUSED", (WindowWidth / 2) - 60, 10, 24, RED);
            }

            DrawText(result.c_str(), WindowWidth - 200, 10, 24, WHITE);
            DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}

std::vector<std::vector<int>> generateWorld(int rows, int cols) {
    std::vector<std::vector<int>> world(rows, std::vector<int>(cols));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            world[i][j] = 0;
        }
    }

    /*
    world[8][2] = 1;
    world[8][3] = 1;
    world[8][4] = 1;

    // Glider
    world[2][7] = 1;
    world[3][7] = 1;
    world[3][5] = 1;
    world[4][7] = 1;
    world[4][6] = 1;
    */

    return world;
}

std::vector<std::vector<int>> simulateWorld(std::vector<std::vector<int>>* world, int worldSize) {
    std::vector<std::vector<int>> newWorld(worldSize, std::vector<int>(worldSize));

    for (int i = 0; i < worldSize; i++) {
        for (int j = 0; j < worldSize; j++) {
            newWorld[i][j] = simulateCell(i, j, world, worldSize, worldSize);
        }
    }

    return newWorld;
}

int simulateCell(int row, int col, std::vector<std::vector<int>>* world, int maxRows, int maxCols) {
    const int currentValue = (*world)[row][col];
    int neighbors = countNeighbors(world, row, col, maxRows);

    /* RULES
    * 1. Any live cell with fewer than two live neighbours dies, as if by underpopulation.
    * 2. Any live cell with two or three live neighbours lives on to the next generation.
    * 3. Any live cell with more than three live neighbours dies, as if by overpopulation.
    * 4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
    */
    if (neighbors == 3 && currentValue == 0) {
        return 1;
    } else if ((neighbors < 2 || neighbors > 3) && currentValue == 1) {
        return 0;
    } else {
        return (*world)[row][col];
    }
}

int countNeighbors(std::vector<std::vector<int>>* world, int row, int col, int worldSize) {
    int counter = 0;
    for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
            if (j == 0 && i == 0) continue; // Skip self

            const int newRow = row + i;
            const int newCol = col + j;

            if (newRow >= 0 && newRow < worldSize - 1 && newCol >= 0 && newCol < worldSize - 1) {
                if ((*world)[row + i][col + j] == 1) {
                    counter++;
                }
            }
        }
    }

    return counter;
}
