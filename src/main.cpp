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

enum Algorithm
{
    Backtrack, OriginShift, AlgorithmSize
};

enum State
{
    Play, Pause, Step
};

struct Cell
{
    int x=0, y=0;
    // Walls
    bool south=false, east=false;
    bool isVisited=false;
    Cell* pointedCell = nullptr;
};


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

void initBacktrack(auto& cells)
{
    // Init cells positions
    for (int y = 0; y < GridHeight; y++)
    {
        for (int x = 0; x < GridWidth; x++)
        {
            Cell& cell = cells[x + y * GridWidth];
            cell.x = x;
            cell.y = y;
            cell.south = true;
            cell.east = true;
        }
    }
}

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
            nextCell->south = false;
            break;
        case East:
            currentCell->east = false;
            break;
        case South:
            currentCell->south = false;
            break;
        case West:
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

void recalculateWalls(auto& cells)
{
    // Init cells positions
    for (int y = 0; y < GridHeight; y++)
    {
        for (int x = 0; x < GridWidth; x++)
        {
            Cell& cell = cells[x + y * GridWidth];
            Cell* pointedCell = cell.pointedCell;
            cell.south = true;
            cell.east = true; 

            // Scip origin cell
            if (pointedCell == nullptr)
            {
                continue;
            }
            
            // West
            if (cell.x > pointedCell->x)
            {
                pointedCell->east = false;
            }
            // East
            else if (cell.x < pointedCell->x)
            {
                cell.east = false;
            }
            // South
            else if (cell.y > pointedCell->y)
            {
                pointedCell->south = false;
            }
            // North
            else if (cell.y < pointedCell->y)
            {
                cell.south = false;
            }
        }
    }
}

// Origin shift
void initOriginShift(auto& cells)
{
    // Init cells positions
    for (int y = 0; y < GridHeight; y++)
    {
        for (int x = 0; x < GridWidth; x++)
        {
            Cell& cell = cells[x + y * GridWidth];
            cell.x = x;
            cell.y = y;
            cell.south = true;
            cell.east = true;
            
            // Last cell is origin
            if (y == GridHeight - 1 && x == GridWidth - 1)
            {
                break;
            }

            if (x != GridWidth - 1)
            {
                cells[x + y * GridWidth].pointedCell = &cells[(x + 1) + y * GridWidth];
            }
            else
            {
                cells[x + y * GridWidth].pointedCell = &cells[x + (y + 1) * GridWidth];
            }
        }
    }

    recalculateWalls(cells);
}

bool tryGetCell(auto& cells, Cell*& currentCell, const int x, const int y)
{
    if (x >= 0 && x < GridWidth && y >= 0 && y < GridHeight)
    {
        currentCell = &cells[x + y * GridWidth];
        return true;
    }

    return false;
}

void originShift(auto& cells, Cell*& currentCell, std::mt19937& mt)
{
    int x = currentCell->x;
    int y = currentCell->y;
    
    std::uniform_int_distribution randomGetter(0, 3);
    int randDir = randomGetter(mt);
    Cell* nextCell = nullptr;
    while (nextCell == nullptr)
    {
        switch (randDir)
        {
        case North:
            if (tryGetCell(cells, nextCell, x, y - 1))
            {
                currentCell->pointedCell = nextCell;
                nextCell->pointedCell = nullptr;
            }
            break;
        case East:
            if (tryGetCell(cells, nextCell, x + 1, y))
            {
                currentCell->pointedCell = nextCell;
                nextCell->pointedCell = nullptr;
            }
            break;
        case South:
            if (tryGetCell(cells, nextCell, x, y + 1))
            {
                currentCell->pointedCell = nextCell;
                nextCell->pointedCell = nullptr;
            }
            break;
        case West:
            if (tryGetCell(cells, nextCell, x - 1, y))
            {
                currentCell->pointedCell = nextCell;
                nextCell->pointedCell = nullptr;
            }
            break;
        default: printf("Random generated unknown direction\n");
        }
        randDir = randomGetter(mt);
    }
    
    currentCell = nextCell;
    recalculateWalls(cells);
}

void init(auto& cells, Cell*& currentCell,  const Algorithm algorithm)
{
    switch (algorithm)
    {
    case Backtrack:
        currentCell = &cells[0];
        initBacktrack(cells);
        break;
    case OriginShift:
        currentCell = &cells[GridHeight*GridWidth - 1];
        initOriginShift(cells);
        break;
    }
}

int main()
{
    using namespace std;

    // Cells
    constexpr int cellsCount = GridWidth * GridHeight;
    array<Cell, cellsCount> cells;
    Cell* currentCell = nullptr;
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
    Algorithm algorithm = OriginShift;
    bool isAlgorithmOpen = false;
    int algorithmActive = algorithm;
    const char* algorithmText = "Backtrack;Origin Shift";
    
    State state = Pause;
    float speed = 1;
    float timer = 0;

    // Init
    init(cells, currentCell, algorithm);
    
    while (!WindowShouldClose())
    {
        // Logic
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
                    switch (algorithm)
                    {
                    case Backtrack:
                        backtrack(cells, backStack, currentCell, mt, state);
                        break;
                    case OriginShift:
                        originShift(cells, currentCell, mt);
                        break;
                    }
                }
            }
            break;
        case Step:
            switch (algorithm)
            {
            case Backtrack:
                backtrack(cells, backStack, currentCell, mt, state);
                break;
            case OriginShift:
                originShift(cells, currentCell, mt);
                break;
            }
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

        constexpr int newSize = CellSize - Border;
        for (const Cell& cell : cells)
        {
            const int x = cell.x * CellSize + Border;
            const int y = cell.y * CellSize + Border;

            Color cellColor;
            switch (algorithm)
            {
            case Backtrack:
                if (cell.x == currentCell->x && cell.y == currentCell->y) cellColor = CurrentColor;
                else if (cell.x == endCell->x && cell.y == endCell->y) cellColor = EndColor;
                else cellColor = cell.isVisited ? VisitedColor : UnvisitedColor;
                break;
            case OriginShift:
                if (cell.x == currentCell->x && cell.y == currentCell->y) cellColor = CurrentColor;
                else cellColor = VisitedColor;
                break;
            }

            // Draw cell
            DrawRectangle(x, y, newSize, newSize, cellColor);

            // Draw walls
            // Right
            DrawRectangle(x + CellSize - Border, y, Border, newSize, cell.east ? WallColor : VisitedColor);
            // Bottom
            DrawRectangle(x, y + CellSize - Border, newSize, Border, cell.south ? WallColor : VisitedColor);
        }

        if (algorithm == OriginShift)
        {
            for (const Cell& cell : cells)
            {
                const int startX = cell.x * CellSize + Border + newSize/2;
                const int startY = cell.y * CellSize + Border + newSize/2;
                    
                DrawCircle(startX, startY, 5.0f, BLACK);
                if(cell.pointedCell != nullptr)
                {
                    const int endX = cell.pointedCell->x * CellSize + Border + newSize/2;
                    const int endY = cell.pointedCell->y * CellSize + Border + newSize/2;
                    DrawLine(startX, startY, endX, endY, BLACK);
                    float c = 10.0f;
                    // DrawTriangle({(float)endX - c, (float)endY + c},{(float)endX, (float)endY},{(float)endX-c, (float)endY-c}, RED);
                }
            }
        }

        // Buttons
        DrawRectangleRec(SidePanel, UnvisitedColor);
        constexpr float gap = SidePanel.height * 0.01f;
        constexpr float x = SidePanel.x + gap;
        constexpr float y = SidePanel.y + gap;
        constexpr float step = SidePanel.height * 0.1f - gap;
        float currStep = 0;

        // Dropdown
        const int ret = GuiDropdownBox({x, y + currStep, SidePanel.width - 2 * gap, step - gap},
            algorithmText, &algorithmActive, isAlgorithmOpen);
        if (ret != 0)
        {
            isAlgorithmOpen = !isAlgorithmOpen;
            if (!isAlgorithmOpen)
            {
                algorithm = static_cast<Algorithm>(algorithmActive);
                init(cells, currentCell, algorithm);
            }
        }
        currStep += isAlgorithmOpen ? step*AlgorithmSize + step : step;
        
        // Start
        if (GuiButton({x, y + currStep, SidePanel.width - 2 * gap, step - gap}, "Start"))
        {
            state = Play;
        }
        currStep += step;

        // Stop
        if (GuiButton({x, y + currStep, SidePanel.width - 2 * gap, step - gap}, "Stop"))
        {
            state = Pause;
        }
        currStep += step;

        // Step
        if (GuiButton({x, y + currStep, SidePanel.width - 2 * gap, step - gap}, "Step"))
        {
            if (state == Pause) state = Step;
        }
        currStep += step;

        // Reset
        if (GuiButton({x, y + currStep, SidePanel.width - 2 * gap, step - gap}, "Reset"))
        {
            state = Pause;
            init(cells, currentCell, algorithm);
        }
        currStep += step;
        
        // Speed Slider
        GuiSetStyle(DEFAULT, TEXT_SIZE, ScreenWidthPx / 25);
        GuiLabel({x, y + currStep, SidePanel.width - 2 * gap, step - 2 * gap}, TextFormat("Speed: %.1f", speed));
        currStep += step;
        GuiSlider({x, y + currStep, SidePanel.width - 2 * gap, step * 0.25f}, "", "", &speed, 1, 60);
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
