#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "styles/dark/style_dark.h"
#include "styles/genesis/style_genesis.h"
#include "styles/amber/style_amber.h"

#define PADDING 8

typedef struct {
    Rectangle panel;
    Rectangle content;
    Vector2 scroll;
} Window;

typedef struct {
    Rectangle panel;
    bool edit;
    int active;
} DropDown;

typedef struct {
    Texture2D texture;
    Rectangle* src;
    Rectangle* dst;
    char* path;
    size_t count;
    size_t dim;
    size_t idx;
} SelectedTexture;

typedef struct {
    Camera2D camera;
    Window window_tile;
    DropDown dropdown_seperator;
    Rectangle panel_top;
    SelectedTexture selected;
    size_t tsize;
    size_t seperator;
} Gui;

Gui gui_init()
{
    Gui gui;

    // Selected Texture;
    gui.selected.texture.id = 0;
    gui.selected.count = 0;
    gui.selected.dim = 0;
    gui.selected.idx = 0;
    gui.selected.src = NULL;
    gui.selected.dst = NULL;

    gui.seperator = 64;

    // 2D Camera
    gui.camera = (Camera2D){0};
    gui.camera.zoom = 1.0f;

    //  Top Panel
    gui.panel_top = (Rectangle){0, 0, GetScreenWidth(), 40};

    // Tile Window
    gui.window_tile = (Window){
        (Rectangle){PADDING, PADDING+40, 320+3, GetScreenHeight()-PADDING-PADDING-40}, // panel
        (Rectangle){PADDING, PADDING+40+RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT, 318, 0}, // content
        (Vector2){0} // scroll
    };

    // Split Drop Down
    gui.dropdown_seperator = (DropDown){(Rectangle){PADDING, PADDING, 120, 24}, false, 0};

    return gui;
}

void selected_update(Gui* gui)
{
    gui->selected.idx = 0;
    // printf("sep %d\n", gui->seperator);
    // This might cause an error if the texture size is not a square and a multiple of 64
    int col, row;
    int d = gui->selected.texture.width/gui->seperator;
    // printf("dim %d\n", d);
    int c = d*d;
    // printf("cnt %d\n", c);
    gui->selected.dim = d;
    gui->selected.count = c;

    gui->selected.src = (Rectangle*)malloc(sizeof(Rectangle) * c);
    if (!gui->selected.src) 
    {
        exit(1);
    }

    gui->selected.dst = (Rectangle*)malloc(sizeof(Rectangle) * c);
    if (!gui->selected.dst) 
    {
        exit(1);
    }

    // Source Rec
    row = 0; 
    col = 0;
    for (int i = 0; i < c; i++)
    {
        if (i%d==0)
        {
            row++;
            col=0;
        }
        gui->selected.src[i] = (Rectangle){gui->seperator*col, gui->seperator*row, gui->seperator, gui->seperator};
        col++;
    }

    // Destination Rec
    row = 0;
    col = 0;
    for (int i = 0; i < c; i++)
    {
        if (i%5==0)
        {
            row++;
            col=0;
        }
        gui->selected.dst[i] = (Rectangle){64*col+PADDING+2, 64*row+PADDING+1, 64, 64};
        col++;
    }

}

void gui_update(Gui* gui)
{
    // Update gui sizes
    gui->panel_top.width = GetScreenWidth();
    gui->window_tile.panel.height = GetScreenHeight()-PADDING-PADDING-40; 
    gui->window_tile.content.height = (int)(ceil((float)gui->selected.count/5.0f)*64.0f);

    if (gui->window_tile.panel.height < gui->window_tile.content.height)
        gui->window_tile.panel.width=320+3+12;
    else
        gui->window_tile.panel.width=320+3;

    // Load Dropped File to selected texutre
    if (IsFileDropped())
    {
        FilePathList l = LoadDroppedFiles();

        const char* e = GetFileExtension(l.paths[0]);
        if (e && (TextIsEqual(e, ".png") || TextIsEqual(e, ".jpg") || TextIsEqual(e, ".jpeg")))
        {
            printf("Loaded File: %s\n", l.paths[0]);

            if (gui->selected.texture.id > 0) UnloadTexture(gui->selected.texture);
            gui->selected.texture = LoadTextureFromImage(LoadImage(l.paths[0]));

            printf("wdt %d\nhig %d\n", gui->selected.texture.width, gui->selected.texture.height);

            selected_update(gui);

            UnloadDroppedFiles(l);
        }
    }

    Vector2 mouse = GetMousePosition();
    Vector2 mouse2d = GetScreenToWorld2D(mouse, gui->camera);

    // move camera
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) 
    {
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -1.0f/gui->camera.zoom);
        gui->camera.target = Vector2Add(gui->camera.target, delta);
    }

    // zoom
    float wheel = GetMouseWheelMove();
    if (wheel != 0) 
    {
        gui->camera.offset = mouse;
        gui->camera.target = mouse2d;
        float scale = 0.2f*wheel;
        gui->camera.zoom = Clamp(expf(logf(gui->camera.zoom)+scale), 0.125f, 64.0f);        
    }

    // reset camera
    if (IsKeyPressed(KEY_R)) 
    {
        gui->camera.target = Vector2Zero();
        gui->camera.offset = Vector2Zero();
        gui->camera.zoom = 1.0;
    }
}

void gui_draw(Gui* gui)
{
    BeginMode2D(gui->camera);
    {
        rlPushMatrix();
        {
            // rlTranslatef(500, 210, 0);
            rlTranslatef(25*32, 25*32, 0);
            rlRotatef(90, 1, 0, 0);
            DrawGrid(50, 32);
        }
        rlPopMatrix();
    }

    // Draw Selected Texture to mouse
    if (gui->selected.texture.id > 0)
    {
        Vector2 m = GetScreenToWorld2D(GetMousePosition(), gui->camera);
        Vector2 nearest = {round((m.x-128)/32)*32, round((m.y-128)/32)*32}; 
        DrawTexturePro(
            gui->selected.texture, // txtr
            gui->selected.src[gui->selected.idx], // src
            (Rectangle){nearest.x, nearest.y, 256, 256}, //dest
            (Vector2){0, 0}, 0, WHITE
        );

        DrawRectangleLines(nearest.x, nearest.y, 256, 256, RED);
    }

    // DrawTexturePro(d, (Rectangle){0, 0, d.width, d.height}, (Rectangle){dst.x, dst.y, d.width, d.height}, Vector2Zero(), 0, WHITE);
    // DrawRectangleLines(dst.x, dst.y, d.width, d.height, RED);
    EndMode2D();

    // Top Panel
    GuiPanel(gui->panel_top, NULL); 

    // Tile Selection Window
    GuiScrollPanel(gui->window_tile.panel, "Window", gui->window_tile.content, &gui->window_tile.scroll, NULL);

    BeginScissorMode(gui->window_tile.panel.x, gui->window_tile.panel.y+RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT, gui->window_tile.panel.width, gui->window_tile.panel.height-3-RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT);
    if (gui->selected.texture.id > 0)
    {
        for (int i = 0; i < gui->selected.count; i++)
        {
            Rectangle d = {gui->selected.dst[i].x, gui->selected.dst[i].y+gui->window_tile.scroll.y, gui->selected.dst[i].height, gui->selected.dst[i].width};
            DrawTexturePro( gui->selected.texture, gui->selected.src[i], d, (Vector2){0, 0}, 0, WHITE);
            DrawRectangleLines(gui->selected.dst[i].x, gui->selected.dst[i].y+gui->window_tile.scroll.y, 64, 64, Fade(WHITE, 0.15f));

            Vector2 m = GetMousePosition();
            if (CheckCollisionPointRec(m, d))
            {
                DrawRectangleRec(d, Fade(WHITE, 0.15f));
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    gui->selected.idx = i;
                }
            }
        }
    }
    EndScissorMode();

    if (gui->dropdown_seperator.edit) GuiLock();
    if (GuiDropdownBox(gui->dropdown_seperator.panel, "64;128;256", &gui->dropdown_seperator.active, gui->dropdown_seperator.edit))
    {
        gui->dropdown_seperator.edit = !gui->dropdown_seperator.edit;
        switch (gui->dropdown_seperator.active)
        {
            case 0: gui->seperator = 64;  break;
            case 1: gui->seperator = 128; break;
            case 2: gui->seperator = 256; break;
        }
        if (gui->selected.texture.id != 0)
            selected_update(gui);
        
    }
    GuiUnlock();

}

void gui_clean(Gui* gui)
{
    // Unload Textures
    UnloadTexture(gui->selected.texture);
    free(gui->selected.src);
    free(gui->selected.dst);
}

int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(800, 600, "window");
    SetTargetFPS(60);

    // GuiLoadStyleGenesis();    
    // GuiLoadStyleDark();
    GuiLoadStyleAmber();

    Gui gui = gui_init();
    char ms[50];

    while (!WindowShouldClose())
    {
        Vector2 m = GetScreenToWorld2D(GetMousePosition(), gui.camera);
        sprintf(ms, "x: %.0f, y: %.0f", m.x, m.y);

        gui_update(&gui);

        BeginDrawing(); ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
            gui_draw(&gui);
            DrawText(ms, GetScreenWidth() - 120, GetScreenHeight() - 20, 18, WHITE);
        EndDrawing();
    }

    gui_clean(&gui);
    CloseWindow();
    return 0;
}