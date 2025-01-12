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

void initCells(auto& cells, const int width, const int height)
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

// Settings
constexpr int GridWidth = 20;
constexpr int GridHeight = 20;
constexpr int CellSize = 75;
constexpr int Border = CellSize / 5;
constexpr Rectangle SidePanel
    {
        .x = GridWidth * CellSize + Border,
        .y = Border,
        .width = CellSize * GridWidth * 0.3f - Border,
        .height = GridHeight * CellSize - Border,
    };
constexpr int ScreenWidthPx = static_cast<int>(GridWidth * CellSize + Border + SidePanel.width + Border);
constexpr int ScreenHeightPx = GridHeight * CellSize + Border;

constexpr auto WallColor = DARKGRAY;
constexpr auto VisitedColor = BLUE;
constexpr auto UnvisitedColor = DARKGREEN;
constexpr auto CurrentColor = RAYWHITE;
constexpr auto EndColor = MAROON;

void addAvailableCell(auto& cells, auto& availableCells, Dir dir, const int nextX, const int nextY)
{
    int index = nextX + nextY * GridWidth;
    if (nextX >= 0 && nextX < GridWidth && nextY >= 0 && nextY < GridHeight && !cells[index].isVisited)
    {
        Cell* availableCell = &cells[index];
        availableCells.emplace_back(availableCell, dir);
    }
}

void backtrack(auto& cells, std::stack<Cell*>& backStack, Cell*& currentCell, std::mt19937& mt, State& state)
{
    constexpr bool isFullBacktrack = false;
    currentCell->isVisited = true;
    
    // Find available cells
    std::vector<std::pair<Cell*, Dir>> availableCells;
    int x = currentCell->x;
    int y = currentCell->y;
    
    addAvailableCell(cells, availableCells, North, x, y - 1);
    addAvailableCell(cells, availableCells, East, x + 1, y);
    addAvailableCell(cells, availableCells, South, x, y + 1);
    addAvailableCell(cells, availableCells, West, x - 1, y);
    
    // Randomly get next cell
    if (!availableCells.empty())
    {
        if (availableCells.size() > 1 || isFullBacktrack) backStack.push(currentCell);

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

    // Cells
    constexpr int cellsCount = GridWidth * GridHeight;
    array<Cell, cellsCount> cells;
    initCells(cells, GridWidth, GridHeight);
    Cell* currentCell = &cells[0];
    const Cell* endCell = &cells[GridWidth * GridHeight - 1];
    stack<Cell*> backStack;

    // Init window
    InitWindow(ScreenWidthPx, ScreenHeightPx, "Maze");
    SetTargetFPS(60);
    GuiSetStyle(DEFAULT, TEXT_SIZE, ScreenWidthPx / 20);
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
                const float sleepTimeS = 1 / speed;
                if (timer < sleepTimeS)
                {
                    timer += GetFrameTime();
                }
                else
                {
                    timer = 0;
                    backtrack(cells, backStack, currentCell, mt, state);
                }
            }
            break;
        case Step:
            backtrack(cells, backStack, currentCell, mt, state);
            state = Pause;
            break;
        case Pause:
        default:
            // Do nothing
            break;
        }

        // Drawing
        BeginDrawing();

        ClearBackground(WallColor);

        for (const Cell& cell : cells)
        {
            const int x = cell.x * CellSize + Border;
            const int y = cell.y * CellSize + Border;
            constexpr int newSize = CellSize - Border;

            Color cellColor;
            if (cell.x == currentCell->x && cell.y == currentCell->y) cellColor = CurrentColor;
            else if (cell.x == endCell->x && cell.y == endCell->y) cellColor = EndColor;
            else cellColor = cell.isVisited ? VisitedColor : UnvisitedColor;

            // Draw cell
            DrawRectangle(x, y, newSize, newSize, cellColor);

            // Draw walls
            // Right
            DrawRectangle(x + CellSize - Border, y, Border, newSize, cell.east ? WallColor : VisitedColor);
            // Bottom
            DrawRectangle(x, y + CellSize - Border, newSize, Border, cell.south ? WallColor : VisitedColor);
        }

        // Buttons
        DrawRectangleRec(SidePanel, UnvisitedColor);
        const float gap = SidePanel.height * 0.01f;
        const float step = SidePanel.height * 0.1f - gap;
        const float x = SidePanel.x + gap;
        const float y = SidePanel.y + gap;
        if (GuiButton({x, y, SidePanel.width - 2 * gap, step - gap}, "Start"))
        {
            state = Play;
        }
        if (GuiButton({x, y + step, SidePanel.width - 2 * gap, step - gap}, "Stop"))
        {
            state = Pause;
        }
        if (GuiButton({x, y + 2 * step, SidePanel.width - 2 * gap, step - gap}, "Step"))
        {
            if (state == Pause) state = Step;
        }
        if (GuiButton({x, y + 3 * step, SidePanel.width - 2 * gap, step - gap}, "Reset"))
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
        GuiSetStyle(DEFAULT, TEXT_SIZE, ScreenWidthPx / 25);
        GuiLabel({x, y + 4 * step, SidePanel.width - 2 * gap, step - 2 * gap}, "Speed:");
        GuiSlider({x, y + 5 * step, SidePanel.width - 2 * gap, step * 0.25f}, "", "", &speed, 1, 60);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
