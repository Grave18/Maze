#include <stack>
#include <cstdlib>
#include <random>
#include <thread>
#include <array>

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

enum Dir
{
    North, East, South, West,
};

enum State
{
    Play, Pause, Step
};

struct Cell
{
    int x = 0, y = 0;
    bool south = true, east = true, north = true, west = true;
    bool isVisited = false;
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

void performStep(auto& cells, std::stack<Cell*>& backStack, Cell*& currentCell, const int gridWidth, const int gridHeight, std::mt19937& mt, State& state)
{
    currentCell->isVisited = true;
    // Find available cells
    std::vector<std::pair<Cell*, Dir>> availableCells;
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
            fprintf(stderr, "ERROR: Index out of range");
        }
    }

    // Randomly get next cell
    if (!availableCells.empty())
    {
        backStack.push(currentCell);
                
        std::uniform_int_distribution randomGetter(0, static_cast<int>(availableCells.size() - 1));
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
    // Backtrack or end
    else
    {
        // Reach end condition
        if (backStack.empty())
        {
            state = Pause;
            printf("Maze generated!\n");
        }
        else
        {
            currentCell = backStack.top();
            backStack.pop();
        }
    }
}

int main()
{
    using namespace std;

    // Settings
    constexpr int gridWidth = 20;
    constexpr int gridHeight = 20;
    constexpr int cellSize = 75;
    constexpr int border = cellSize / 5;
    constexpr float sleepTimeS = 1;
    
    Rectangle panel
    {
        .x = gridWidth*cellSize + border,
        .y = border,
        .width = cellSize*gridWidth*0.3f - border,
        .height = gridHeight*cellSize - border,
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
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0x383838FF);

    // Init random
    random_device randomDevice;
    mt19937 mt(randomDevice());

    // Start cycle
    State state = Pause;
    float speed = 1;
    float timer = 0;
    while (!WindowShouldClose())
    {
        switch (state)
        {
        case Play:
            {
                // Sleep for
                float finaleTimeS = sleepTimeS/speed;
                if (timer < finaleTimeS)
                {
                    timer += GetFrameTime();
                }
                else
                {
                    timer = 0;
                    performStep(cells, backStack, currentCell, gridWidth, gridHeight, mt, state);
                }
            }
            break;
        case Step:
            performStep(cells, backStack, currentCell, gridWidth, gridHeight, mt, state);
            state = Pause;
            break;
        case Pause:
        default:
            // Do nothing
            break;
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
                state = Play;
            }
            if (GuiButton({x, y + step, panel.width - 2*gap, step - 2*gap}, "Stop"))
            {
                state = Pause;
            }
            if (GuiButton({x, y + 2*step, panel.width - 2*gap, step - 2*gap}, "Step"))
            {
                if (state == Pause) state = Step;
            }
            if (GuiButton({x, y + 3*step, panel.width - 2*gap, step - 2*gap}, "Reset"))
            {
                state = Pause;
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
            GuiSetStyle(DEFAULT, TEXT_SIZE, screenWidth / 25);
            GuiLabel( {x, y + 4*step, panel.width - 2*gap, step - 2*gap}, "Speed:");
            GuiSlider({x, y + 5*step, panel.width - 2*gap, step*0.25f}, "", "", &speed, 1, 60);
            
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
