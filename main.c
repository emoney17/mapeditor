#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <raymath.h>

typedef struct {
    int dim;
    int size;
} grid;

void cameraupdate(Camera2D* c) {
    Vector2 m = GetMousePosition();
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -1.0f/c->zoom);
        c->target = Vector2Add(c->target, delta);
    }

    float w = GetMouseWheelMove();
    Vector2 m2d = GetScreenToWorld2D(m, *c);
    if (w != 0) {
        c->offset = m;
        c->target = m2d;
        float scale = 0.2f*w;
        c->zoom = Clamp(expf(logf(c->zoom)+scale), 0.125f, 64.0f);        
    }

    if (IsKeyPressed(KEY_R)) {
        c->target = Vector2Zero();
        c->offset = Vector2Zero();
        c->zoom = 1.0;
    }
}

int main() {
    InitWindow(1000, 1000, NULL);
    SetTargetFPS(60);

    Camera2D camera;
    camera        = (Camera2D){0};
    camera.zoom   = 1.0f;
    camera.target = (Vector2){GetScreenWidth()/2.0f, GetScreenHeight()/2.0f};
    camera.offset = (Vector2){GetScreenWidth()/2.0f, GetScreenHeight()/2.0f};

    int dim = 100;
    int size = 64;
    Vector2 x1 = Vector2Zero(), x2 = Vector2Zero(), y1 = Vector2Zero(), y2 = Vector2Zero();

    while (!WindowShouldClose()) {
        cameraupdate(&camera);
        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);
        for (int i = 0; i <= dim*size; i+=size) {
            x1 = (Vector2){i, 0};
            y1 = (Vector2){i, dim*size};

            x2 = (Vector2){0, i};
            y2 = (Vector2){dim*size, i};

            DrawLineV(x1, y1, Fade(WHITE, 0.125f));
            DrawLineV(x2, y2, Fade(WHITE, 0.125f));

            // DrawLine(i, 0, i, dim*size, Fade(WHITE, 0.125f));
            // DrawLine(0, i, dim*size, i, Fade(WHITE, 0.125f));
        }
        EndMode2D();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}