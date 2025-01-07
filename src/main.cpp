#include <stack>
#include <cstdlib>
#include <random>
#include <thread>

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <array>
#include <raygui.h>

enum Dir
{
    North, East, South, West,
};

struct Cell
{
    int x = 0, y = 0;
    bool south = true, east = true, north = true, west = true;
    bool isVisited = false;
    Color color = BLUE;
};

void initCells(auto &cells, const int width, const int height)
{
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
}

void addAvailableCell(auto& cells, auto& availableCells, Dir dir, const int nextX, const int nextY, const int gridWidth, const int gridHeight)
{
    int index = nextX + nextY * gridWidth;
    if (nextX >= 0 && nextX < gridWidth && nextY >= 0 && nextY < gridHeight && !cells[index].isVisited)
    {
        Cell* availableCell = &cells[index];
        availableCells.emplace_back(availableCell, dir);
    }
}

int main()
{
    using namespace std;

    // Settings
    constexpr int gridWidth = 25;
    constexpr int gridHeight = 25;
    constexpr int cellSize = 60;
    constexpr int border = cellSize / 5;
    constexpr float sleepTimeS = 1;
    
    Rectangle panel
    {
        .x = gridWidth*cellSize + border,
        .y = border,
        .width = cellSize*6 - border,
        .height = gridHeight*cellSize - border
    };

    constexpr auto wallColor = DARKGRAY;
    constexpr auto visitedColor = BLUE;
    constexpr auto unvisitedColor = DARKGREEN;
    constexpr auto currentColor = RAYWHITE;
    constexpr auto endColor = MAROON;

    // Cells
    constexpr int cellsCount = gridWidth*gridHeight;
    array<Cell, cellsCount> cells;
    initCells(cells, gridWidth, gridHeight);
    Cell* currentCell = &cells[0];
    const Cell* endCell = &cells[gridWidth*gridHeight-1];
    stack<Cell*> backStack;

    // Init window
    int screenWidth = static_cast<int>(gridWidth*cellSize + border + panel.width + border);
    int screenHeight = gridHeight*cellSize+border;
    InitWindow(screenWidth, screenHeight, "Maze");
    SetTargetFPS(60);
    GuiSetStyle(DEFAULT, TEXT_SIZE, screenWidth / 20);

    // Init random
    random_device randomDevice;
    mt19937 mt(randomDevice());

    // Start cycle
    bool isEndReached = false;
    bool isStart = false;
    bool isStep = false;
    float speed = 1;
    float timer = 0;
    while (!WindowShouldClose())
    {
        // Sleep for
        float finaleTimeS = sleepTimeS/speed;
        
        if (timer < finaleTimeS && !isStep)
        {
            timer += GetFrameTime();
        }
        else if ((isStart && !isEndReached) || isStep)
        {
            timer = 0;
            isStep = false;
        
            currentCell->isVisited = true;
            // Find available cells
            vector<pair<Cell*, Dir>> availableCells;
            int x = currentCell->x;
            int y = currentCell->y;
            for (int dir = 0; dir < 4; dir++)
            {
                switch (dir)
                {
                case North:
                    addAvailableCell(cells, availableCells, North, x, y - 1, gridWidth, gridHeight);
                    break;
                case East:
                    addAvailableCell(cells, availableCells, East, x + 1, y, gridWidth, gridHeight);
                    break;
                case South:
                    addAvailableCell(cells, availableCells, South, x, y + 1, gridWidth, gridHeight);
                    break;
                case West:
                    addAvailableCell(cells, availableCells, West, x - 1, y, gridWidth, gridHeight);
                    break;
                default:
                    printf("ERROR: Index out of range");
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
                    printf("Maze generated!\n");
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
                const int x = cell.x * cellSize + border;
                const int y = cell.y * cellSize + border;
                constexpr int newSize = cellSize - border;
                
                Color cellColor;
                if (cell.x == currentCell->x && cell.y == currentCell->y) cellColor = currentColor;
                else if (cell.x == endCell->x && cell.y == endCell->y) cellColor = endColor;
                else cellColor = cell.isVisited ? visitedColor : unvisitedColor;
                
                // Draw cell
                DrawRectangle(x, y, newSize, newSize, cellColor);

                // Draw walls
                // Right
                DrawRectangle(x + cellSize - border, y, border, newSize, cell.east ? wallColor : visitedColor);
                // Bottom
                DrawRectangle(x, y + cellSize - border, newSize, border, cell.south ? wallColor : visitedColor);
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
            if (GuiSlider({x, y + 4*step, panel.width - 2*gap, step*0.25f}, "", "", &speed, 1, 60))
            {
                // Empty
            }
            
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
