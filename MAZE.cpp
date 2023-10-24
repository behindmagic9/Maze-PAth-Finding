#include <iostream>
#include <SDL2/SDL.h>
#include <vector>
#include <queue>
#include <stack>
#include <cmath>
#include <cstdlib>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int CELL_SIZE = 20;
const int MAZE_WIDTH = (WINDOW_WIDTH / CELL_SIZE);
const int MAZE_HEIGHT = (WINDOW_HEIGHT / CELL_SIZE);

// Structure representing a cell in the maze
struct Cell {
    bool visited;
    bool walls[4]; // Top, Right, Bottom, Left walls of the cell
    int parentX;
    int parentY;
    double gCost;
    double hCost;

    Cell() {
        visited = false;
        for (int i = 0; i < 4; i++) {
            walls[i] = true;
        }
        parentX = -1;
        parentY = -1;
        gCost = INFINITY;
        hCost = 0.0;
    }
};

// SDL variables
SDL_Window* window;
SDL_Renderer* renderer;

std::vector<Cell> maze;
int startX = -1;
int startY = -1;
int endX = -1;
int endY = -1;

// Initialize SDL
bool initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Maze Solver", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cout << "Window creation failed: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cout << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    return true;
}

// Generate the maze using recursive backtracking algorithm
void generateMaze(int x, int y) {
    maze[y * MAZE_WIDTH + x].visited = true;	// y * MAZE_WIDTH + x formula used to get the cell , here it is used to convert the 2d array into the 1d array
    // y be row number (vertical position) of the cell.
	// x be the column number (horizontal position) of the cell.
	// Maze_width = = (WINDOW_WIDTH / CELL_SIZE);

    while (true) {
        std::vector<int> neighbors;

        if (x > 0 && !maze[y * MAZE_WIDTH + (x - 1)].visited) {
            neighbors.push_back(0); // Left neighbor
        }
        
        if (y > 0 && !maze[(y - 1) * MAZE_WIDTH + x].visited) {
            neighbors.push_back(1); // Top neighbor
        }
        
        if (x < MAZE_WIDTH - 1 && !maze[y * MAZE_WIDTH + (x + 1)].visited) {
            neighbors.push_back(2); // Right neighbor
        }
        
        if (y < MAZE_HEIGHT - 1 && !maze[(y + 1) * MAZE_WIDTH + x].visited) {
            neighbors.push_back(3); // Bottom neighbor
        }
	
        if (neighbors.empty()) {
            return;
        }
	
        int randomIndex = rand() % neighbors.size();
        std::cout << randomIndex << std::endl;
        int direction = neighbors[randomIndex];

        if (direction == 0) {
            maze[y * MAZE_WIDTH + x].walls[3] = false;
            maze[y * MAZE_WIDTH + (x - 1)].walls[1] = false;
            generateMaze(x - 1, y);
        } else if (direction == 1) { 
            maze[y * MAZE_WIDTH + x].walls[0] = false;
            maze[(y - 1) * MAZE_WIDTH + x].walls[2] = false;
            generateMaze(x, y - 1);
        } else if (direction == 2) {
            maze[y * MAZE_WIDTH + x].walls[1] = false;
            maze[y * MAZE_WIDTH + (x + 1)].walls[3] = false;
            generateMaze(x + 1, y);
        } else if (direction == 3) {
            maze[y * MAZE_WIDTH + x].walls[2] = false;
            maze[(y + 1) * MAZE_WIDTH + x].walls[0] = false;
            generateMaze(x, y + 1);
        }
    }
}

// Calculate the heuristic cost (Manhattan distance) from a cell to the end point
double calculateHeuristicCost(int x, int y) {
    return std::abs(x - endX) + std::abs(y - endY);
}

// Perform A* algorithm to find the shortest path
bool findShortestPath() {
    std::priority_queue<std::pair<double, std::pair<int, int>>, std::vector<std::pair<double, std::pair<int, int>>>, std::greater<std::pair<double, std::pair<int, int>>>> openSet;
    std::vector<std::vector<bool>> closedSet(MAZE_HEIGHT, std::vector<bool>(MAZE_WIDTH, false));

    int startXIndex = startX / CELL_SIZE;
    int startYIndex = startY / CELL_SIZE;
    int endXIndex = endX / CELL_SIZE;
    int endYIndex = endY / CELL_SIZE;

    openSet.push(std::make_pair(0.0, std::make_pair(startXIndex, startYIndex)));
    maze[startYIndex * MAZE_WIDTH + startXIndex].gCost = 0.0;

    while (!openSet.empty()) {
        int currentX = openSet.top().second.first;
        int currentY = openSet.top().second.second;
        openSet.pop();

        if (currentX == endXIndex && currentY == endYIndex) {
            return true; // Path found
        }

        closedSet[currentY][currentX] = true;

        // Process neighboring cells
        for (int i = 0; i < 4; i++) {
            int nextX = currentX;
            int nextY = currentY;

            if (i == 0 && nextX > 0 && !maze[currentY * MAZE_WIDTH + (currentX - 1)].walls[1]) {
                nextX--;
            } else if (i == 1 && nextY > 0 && !maze[(currentY - 1) * MAZE_WIDTH + currentX].walls[2]) {
                nextY--;
            } else if (i == 2 && nextX < MAZE_WIDTH - 1 && !maze[currentY * MAZE_WIDTH + (currentX + 1)].walls[3]) {
                nextX++;
            } else if (i == 3 && nextY < MAZE_HEIGHT - 1 && !maze[(currentY + 1) * MAZE_WIDTH + currentX].walls[0]) {
                nextY++;
            }

            if (!closedSet[nextY][nextX]) {
                double tentativeGCost = maze[currentY * MAZE_WIDTH + currentX].gCost + 1.0;
                if (tentativeGCost < maze[nextY * MAZE_WIDTH + nextX].gCost) {
                    maze[nextY * MAZE_WIDTH + nextX].parentX = currentX;
                    maze[nextY * MAZE_WIDTH + nextX].parentY = currentY;
                    maze[nextY * MAZE_WIDTH + nextX].gCost = tentativeGCost;
                    maze[nextY * MAZE_WIDTH + nextX].hCost = calculateHeuristicCost(nextX, nextY);

                    double fCost = tentativeGCost + maze[nextY * MAZE_WIDTH + nextX].hCost;
                    openSet.push(std::make_pair(fCost, std::make_pair(nextX, nextY)));
                }
            }
        }
    }

    return false; // Path not found
}

// Render the maze on the screen
void renderMaze() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            int cellIndex = y * MAZE_WIDTH + x;
            int cellX = x * CELL_SIZE;
            int cellY = y * CELL_SIZE;

            if (maze[cellIndex].walls[0]) {
                SDL_RenderDrawLine(renderer, cellX, cellY, cellX + CELL_SIZE, cellY);
            }
            if (maze[cellIndex].walls[1]) {
                SDL_RenderDrawLine(renderer, cellX + CELL_SIZE, cellY, cellX + CELL_SIZE, cellY + CELL_SIZE);
            }
            if (maze[cellIndex].walls[2]) {
                SDL_RenderDrawLine(renderer, cellX + CELL_SIZE, cellY + CELL_SIZE, cellX, cellY + CELL_SIZE);
            }
            if (maze[cellIndex].walls[3]) {
                SDL_RenderDrawLine(renderer, cellX, cellY + CELL_SIZE, cellX, cellY);
            }
        }
    }

    if (startX != -1 && startY != -1) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect startRect{ startX, startY, CELL_SIZE, CELL_SIZE };
        SDL_RenderFillRect(renderer, &startRect);
    }

    if (endX != -1 && endY != -1) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect endRect{ endX, endY, CELL_SIZE, CELL_SIZE };
        SDL_RenderFillRect(renderer, &endRect);
    }

    if (findShortestPath()) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

        int currentX = endX / CELL_SIZE;
        int currentY = endY / CELL_SIZE;

        while (currentX != -1 && currentY != -1) {
            int parentX = maze[currentY * MAZE_WIDTH + currentX].parentX;
            int parentY = maze[currentY * MAZE_WIDTH + currentX].parentY;

            int lineX1 = currentX * CELL_SIZE + CELL_SIZE / 2;
            int lineY1 = currentY * CELL_SIZE + CELL_SIZE / 2;
            int lineX2 = parentX * CELL_SIZE + CELL_SIZE / 2;
            int lineY2 = parentY * CELL_SIZE + CELL_SIZE / 2;

            SDL_RenderDrawLine(renderer, lineX1, lineY1, lineX2, lineY2);

            currentX = parentX;
            currentY = parentY;
        }
    }

    SDL_RenderPresent(renderer);
}

// Free resources and quit SDL
void quitSDL() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    if (!initSDL()) {
        return 1;
    }

    maze.resize(MAZE_WIDTH * MAZE_HEIGHT);

    generateMaze(0, 0);
    renderMaze();

    SDL_Event e;
    bool quit = false;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = e.button.x;
                int mouseY = e.button.y;

                if (mouseX >= 0 && mouseX < WINDOW_WIDTH && mouseY >= 0 && mouseY < WINDOW_HEIGHT) {
                    int cellX = (mouseX / CELL_SIZE) * CELL_SIZE;
                    int cellY = (mouseY / CELL_SIZE) * CELL_SIZE;

                    if (startX == -1 && startY == -1) {
                        startX = cellX;
                        startY = cellY;
                    } else if (endX == -1 && endY == -1 && (cellX != startX || cellY != startY)) {
                        endX = cellX;
                        endY = cellY;

                        renderMaze();
                    }
                }
            }
        }
    }

    quitSDL();
    return 0;
}

