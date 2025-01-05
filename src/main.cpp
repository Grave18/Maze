#include <raylib.h>

int main()
{
    InitWindow(1920, 1080, "Maze");
    
    while(!WindowShouldClose())
    {
        BeginDrawing();
        DrawRectangle(100, 100, 100, 100, RED);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}