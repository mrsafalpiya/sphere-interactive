#include <stdio.h>
#include <math.h>

/* raylib-nuklear -- https://github.com/RobLoach/raylib-nuklear */
#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "dep/raylib-nuklear/raylib-nuklear.h"

/* raylib extra */
#include <rlgl.h>

// Window Properties
// -----------------------------------------------------------------------------

#define WIN_WIDTH     800
#define WIN_HEIGHT    600
#define WIN_FONT_SIZE 10

// Render Properties
// -----------------------------------------------------------------------------

#define BG_COL     DARKGRAY
#define SPHERE_COL RAYWHITE

// Info Text
// -----------------------------------------------------------------------------

#define INFO_TEXT                            \
	"Click and move the mouse to rotate" \
	"\n"                                 \
	"Scroll up/down to zoom in/out"      \
	"\n"                                 \
	"Arrow keys to move"                 \
	"\n"                                 \
	"\n"                                 \
	"S - Settings"                       \
	"\n"                                 \
	"Q - Exit"
#define INFO_FG   SPHERE_COL
#define INFO_SIZE 20

float
map(float p, float p_min, float p_max, float a, float b)
{
	return a + (b - a) * ((p - p_min) / (p_max - p_min));
}

void
sphere_fill_points(int detail, float radius, Vector3 points[detail * detail])
{
	int c = 0;

	for (int i = 0; i < detail; i++) {
		float lat = map(i, 0, detail, -PI / 2, PI / 2);
		for (int j = 0; j < detail; j++) {
			float lon = map(j, 0, detail, -PI, PI);

			float x = radius * cos(lon) * cos(lat);
			float y = radius * cos(lon) * sin(lat);
			float z = radius * sin(lon);

			points[c].x = x;
			points[c].y = y;
			points[c].z = z;
			c++;
		}
	}
}

int
main(void)
{
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(WIN_WIDTH, WIN_HEIGHT, "Sphere Interactive");

	Camera3D camera   = { 0 };
	camera.position   = (Vector3){ 0.0f, 0.0f, -20.0f };
	camera.target     = (Vector3){ 0.0f, 0.0f, 0.0f };
	camera.up         = (Vector3){ 0.0f, 1.0f, 0.0f };
	camera.fovy       = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	int     detail    = 100;
	float   sp_radius = 4.0f;
	Vector3 sp_points[detail * detail];
	sphere_fill_points(detail, sp_radius, sp_points);

	SetTargetFPS(60);

	struct nk_context *nk_ctx = InitNuklear(WIN_FONT_SIZE);

	// Render Settings
	// ---------------------------------------------------------------------
	int   to_display_fps     = 1;
	int   to_display_details = 1;
	int   to_auto_rotate     = 1;
	float rotx               = 0.0f;
	float roty               = 0.0f;
	float posx               = 0.0f;
	float posy               = 0.0f;
	float posz               = 0.0f;

	int to_show_settings = 0;

	while (!WindowShouldClose()) {
		UpdateNuklear(nk_ctx);

		if (IsKeyPressed(KEY_S))
			to_show_settings = 1;
		if (IsKeyPressed(KEY_Q))
			break;
		if (IsKeyDown(KEY_UP))
			posy -= 0.1f;
		if (IsKeyDown(KEY_DOWN))
			posy += 0.1f;
		if (IsKeyDown(KEY_LEFT))
			posx -= 0.1f;
		if (IsKeyDown(KEY_RIGHT))
			posx += 0.1f;

		// Nuklear GUI Code
		// -------------------------------------------------------------
		if (nk_begin(nk_ctx, "Settings", nk_rect(100, 100, 160, 265),
		             NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE |
		                     NK_WINDOW_NO_SCROLLBAR)) {
			nk_layout_row_dynamic(nk_ctx, 20, 2);
			nk_checkbox_label(nk_ctx, "Show FPS", &to_display_fps);
			nk_checkbox_label(nk_ctx, "Show Info",
			                  &to_display_details);

			nk_label(nk_ctx, "Zoom level", NK_TEXT_LEFT);
			nk_layout_row_dynamic(nk_ctx, 20, 1);
			nk_slider_float(nk_ctx, -14.0, &posz, 20.0, 0.1);

			nk_layout_row_dynamic(nk_ctx, 20, 1);
			nk_label(nk_ctx, "Position", NK_TEXT_LEFT);

			nk_button_push_behavior(nk_ctx, NK_BUTTON_REPEATER);
			{
				nk_layout_row_dynamic(nk_ctx, 25, 5);
				nk_spacing(nk_ctx, 2);
				if (nk_button_symbol(nk_ctx,
				                     NK_SYMBOL_TRIANGLE_UP))
					posy -= 0.1f;

				nk_layout_row_dynamic(nk_ctx, 25, 5);
				nk_spacing(nk_ctx, 1);
				if (nk_button_symbol(nk_ctx,
				                     NK_SYMBOL_TRIANGLE_LEFT))
					posx -= 0.1f;
				nk_spacing(nk_ctx, 1);
				if (nk_button_symbol(nk_ctx,
				                     NK_SYMBOL_TRIANGLE_RIGHT))
					posx += 0.1f;

				nk_layout_row_dynamic(nk_ctx, 25, 5);
				nk_spacing(nk_ctx, 2);
				if (nk_button_symbol(nk_ctx,
				                     NK_SYMBOL_TRIANGLE_DOWN))
					posy += 0.1f;
			}
			nk_button_pop_behavior(nk_ctx);

			nk_layout_row_dynamic(nk_ctx, 20, 1);
			nk_checkbox_label(nk_ctx, "Auto rotate",
			                  &to_auto_rotate);
			nk_layout_row_dynamic(nk_ctx, 20, 1);
			if (nk_button_label(nk_ctx,
			                    "Reset camera and position"))
				rotx = roty = posx = posy = posz = 0.0f;
		}

		int to_rotate = 1;
		int to_zoom   = 1;
		if (nk_window_is_hovered(nk_ctx) && to_show_settings) {
			to_rotate = 0;
			to_zoom   = 0;
		}
		nk_end(nk_ctx);

		// Zoom
		// -------------------------------------------------------------
		float zoomFactor = -GetMouseWheelMove();
		if (to_zoom &&
		    (posz - zoomFactor > -14.0 && posz - zoomFactor < 20.0)) {
			posz -= zoomFactor;
		}

		// Rotation
		// -------------------------------------------------------------
		float dt = GetFrameTime();

		if (to_rotate && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			Vector2 mouseDelta = GetMouseDelta();
			roty += 0.2 * mouseDelta.x;
			rotx -= 0.2 * mouseDelta.y;
		} else {
			if (to_auto_rotate)
				roty -= 3 * dt;
		}

		BeginDrawing();
		{
			ClearBackground(BG_COL);

			BeginMode3D(camera);
			{
				// Draw Sphere
				// ---------------------------------------------
				rlTranslatef(posx, posy, -posz);
				rlRotatef(rotx, 1, 0, 0);
				rlRotatef(roty, 0, 1, 0);
				for (int i = 0; i < detail * detail; i++)
					DrawPoint3D(sp_points[i], SPHERE_COL);
			}
			EndMode3D();

			// Draw Details
			// -----------------------------------------------------
			if (to_display_fps)
				DrawFPS(5, 5);
			if (to_display_details) {
				DrawText(INFO_TEXT, 5, 30, INFO_SIZE, INFO_FG);
			}

			if (to_show_settings)
				DrawNuklear(nk_ctx);
			else
				nk_clear(nk_ctx);
		}
		EndDrawing();

		if (nk_window_is_hidden(nk_ctx, "Settings"))
			to_show_settings = 0;
	}

	UnloadNuklear(nk_ctx);
	CloseWindow();
	return 0;
}
