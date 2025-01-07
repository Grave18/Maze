#include <raylib.h>
#include <stack>
#include <cstdlib>
#include <random>
#include <thread>

enum Dir
{
    North, East, South, West,
};

class Cell
{
public:
    int x = 0, y = 0;
    bool south = true, east = true, north = true, west = true;
    bool isVisited = false;
    Color color = BLUE;
};

int main()
{
    using namespace std;
    
    constexpr int size = 100;
    constexpr int border = size / 10;
    constexpr int width = 10;
    constexpr int height = 10;
    
    Cell cells[width * height];
    Cell* currentCell = &cells[0];
    const Cell* endCell = &cells[width*height-1];
    stack<Cell*> backStack;

    // Init cells positions
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            Cell& cell = cells[x + y * width];
            cell.x = x;
            cell.y = y;
        }
    }

    InitWindow(1920, 1080, "Maze");
    SetTargetFPS(60);
    random_device randomDevice;
    mt19937 mt(randomDevice());

    bool isEndReached = false;
    while (!WindowShouldClose())
    {
        if (!isEndReached)
        {
            // Sleep for
            this_thread::sleep_for(chrono::milliseconds(100));
        
            currentCell->isVisited = true;
            // Find available cells
            vector<pair<Cell*, Dir>> availableCells;
            for (int dir = 0; dir < 4; dir++)
            {
                int index;
                switch (dir)
                {
                case North:
                    {
                        const int nextX = currentCell->x;
                        const int nextY = currentCell->y - 1;
                        index = nextX + nextY * width;
                        if (nextX >= 0 && nextX < width && nextY >= 0 && nextY < height && !cells[index].isVisited)
                        {
                            Cell* availableCell = &cells[index];
                            availableCells.emplace_back(availableCell, North);
                        }
                    }
                    break;
                case East:
                    {
                        const int nextX = currentCell->x + 1;
                        const int nextY = currentCell->y;
                        index = nextX + nextY * width;
                        if (nextX >= 0 && nextX < width && nextY >= 0 && nextY < height && !cells[index].isVisited)
                        {
                            Cell* availableCell = &cells[index];
                            availableCells.emplace_back(availableCell, East);
                        }
                    }
                    break;
                case South:
                    {
                        const int nextX = currentCell->x;
                        const int nextY = currentCell->y + 1;
                        index = nextX + nextY * width;
                        if (nextX >= 0 && nextX < width && nextY >= 0 && nextY < height && !cells[index].isVisited)
                        {
                            Cell* availableCell = &cells[index];
                            availableCells.emplace_back(availableCell, South);
                        }
                    }
                    break;
                case West:
                    {
                        const int nextX = currentCell->x - 1;
                        const int nextY = currentCell->y;
                        index = nextX + nextY * width;
                        if (nextX >= 0 && nextX < width && nextY >= 0 && nextY < height && !cells[index].isVisited)
                        {
                            Cell* availableCell = &cells[index];
                            availableCells.emplace_back(availableCell, West);
                        }
                    }
                    break;
                default:
                    printf("ERROR: Index out of range");
                    index = -1;
                }
            }

            // Backtrack or end
            if (!availableCells.empty())
            {
                backStack.push(currentCell);
                
                uniform_int_distribution randomGetter(0, static_cast<int>(availableCells.size() - 1));
                const int randVal = randomGetter(mt);
                auto [nextCell, direction] = availableCells[randVal];

                // Break walls
                switch (direction)
                {
                case North:
                    currentCell->north = false;
                    nextCell->south = false;
                    break;
                case East:
                    currentCell->east = false;
                    nextCell->west = false;
                    break;
                case South:
                    currentCell->south = false;
                    nextCell->north = false;
                    break;
                case West:
                    currentCell->west = false;
                    nextCell->east = false;
                    break;
                }
            
                // Set current cell
                currentCell = nextCell;
            }
            // Randomly get next cell
            else
            {
                // Reach end condition
                if (backStack.empty())
                {
                    isEndReached = true;
                    printf("Reach end!\n");
                    continue;
                }
                
                currentCell = backStack.top();
                backStack.pop();
            }
        }
        
        // Drawing
        BeginDrawing();
        
            ClearBackground(BLACK);
        
            for(const Cell& cell: cells)
            {
                const int x = cell.x * size;
                const int y = cell.y * size;
                constexpr auto wallColor = DARKGRAY;
                constexpr auto visitedColor = BLUE;
                constexpr auto unvisitedColor = DARKGREEN;
                constexpr auto currentColor = RAYWHITE;
                constexpr auto endColor = MAROON;
                
                // Draw cell
                Color cellColor;
                if (cell.x == currentCell->x && cell.y == currentCell->y) cellColor = currentColor;
                else if (cell.x == endCell->x && cell.y == endCell->y) cellColor = endColor;
                else cellColor = cell.isVisited ? visitedColor : unvisitedColor;
                DrawRectangle(x, y, size, size, cellColor);

                // Draw walls
                // Left
                DrawRectangle(x, y, border, size, cell.west ? wallColor : visitedColor);
                // Top
                DrawRectangle(x, y, size, border, cell.north ? wallColor : visitedColor);
                // Right
                DrawRectangle(x + size - border, y, border, size, cell.east ? wallColor : visitedColor);
                // Bottom
                DrawRectangle(x, y + size - border, size, border, cell.south ? wallColor : visitedColor);
            }
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
