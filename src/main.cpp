#include <stack>
#include <cstdlib>
#include <random>
#include <thread>

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

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

    // Settings
    constexpr int width = 15;
    constexpr int height = 15;
    constexpr int size = 100;
    constexpr int border = size / 5;
    constexpr int sleepTimeMs = 1000;
    
    Rectangle panel {width*size+border, border, size*6-border, height*size-border};

    constexpr auto wallColor = DARKGRAY;
    constexpr auto visitedColor = BLUE;
    constexpr auto unvisitedColor = DARKGREEN;
    constexpr auto currentColor = RAYWHITE;
    constexpr auto endColor = MAROON;

    // Cells
    Cell cells[width * height];
    Cell* currentCell = &cells[0];
    const Cell* endCell = &cells[width*height-1];
    stack<Cell*> backStack;

    int screenWidth = static_cast<int>(width * size + border + panel.width + border);
    int screenHeight = height*size+border;
    // Init window
    InitWindow(screenWidth, screenHeight, "Maze");
    SetTargetFPS(60);
    GuiSetStyle(DEFAULT, TEXT_SIZE, screenWidth / 20);
    
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

    // Init random
    random_device randomDevice;
    mt19937 mt(randomDevice());

    // Start cycle
    bool isEndReached = false;
    bool isStart = false;
    bool isStep = false;
    float speedValue = 1;
    float timer = 0;
    while (!WindowShouldClose())
    {
        if ((isStart && !isEndReached) || isStep)
        {
            isStep = false;
            // Sleep for
            int finaleTimeMs = sleepTimeMs/static_cast<int>(speedValue);
            this_thread::sleep_for(chrono::milliseconds(finaleTimeMs));
        
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
        
            ClearBackground(wallColor);
        
            for(const Cell& cell: cells)
            {
                const int x = cell.x * size + border;
                const int y = cell.y * size + border;
                constexpr int newSize = size - border;
                
                Color cellColor;
                if (cell.x == currentCell->x && cell.y == currentCell->y) cellColor = currentColor;
                else if (cell.x == endCell->x && cell.y == endCell->y) cellColor = endColor;
                else cellColor = cell.isVisited ? visitedColor : unvisitedColor;
                
                // Draw cell
                DrawRectangle(x, y, newSize, newSize, cellColor);

                // Draw walls
                // Right
                DrawRectangle(x + size - border, y, border, newSize, cell.east ? wallColor : visitedColor);
                // Bottom
                DrawRectangle(x, y + size - border, newSize, border, cell.south ? wallColor : visitedColor);
            }
        
            // Buttons
            DrawRectangleRec(panel, unvisitedColor);
            const float step = panel.height * 0.1f;
            const float gap = panel.height * 0.01f;
            float x = panel.x + gap;
            float y = panel.y + gap;
            if (GuiButton({x, y, panel.width - 2*gap, step - 2*gap}, "Start"))
            {
                isStart = true;
            }
            if (GuiButton({x, y + step, panel.width - 2*gap, step - 2*gap}, "Stop"))
            {
                isStart = false;
            }
            if (GuiButton({x, y + 2*step, panel.width - 2*gap, step - 2*gap}, "Step"))
            {
                isStep = true;
            }
            if (GuiButton({x, y + 3*step, panel.width - 2*gap, step - 2*gap}, "Reset"))
            {
                isEndReached = false;
                isStart = false;
                isStep = false;
                currentCell = &cells[0];
                backStack = stack<Cell*>();
                for (Cell& cell : cells)
                {
                    cell.north = true;
                    cell.east = true;
                    cell.south = true;
                    cell.east = true;
                    cell.isVisited = false;
                }
            }
            if (GuiSlider({x, y + 4*step, panel.width - 2*gap, step*0.25f}, "", "", &speedValue, 1, 10))
            {
                
            }
            
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
