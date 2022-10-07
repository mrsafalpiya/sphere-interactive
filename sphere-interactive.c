#include <raylib.h>
#include <rlgl.h>
#include <math.h>

// Window Properties
// -----------------------------------------------------------------------------

#define WIN_WIDTH  800
#define WIN_HEIGHT 600

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
	"\n"                                 \
	"R - Toggle auto rotate"             \
	"\n"                                 \
	"I - Toggle this info"               \
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
		float lat = map(i, 0, detail, -PI/2, PI/2);
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

	// Render Settings
	// ---------------------------------------------------------------------
	int   toDisplayDetails = 1;
	int   toAutoRotate     = 1;
	float rotx             = 0.0f;
	float roty             = 0.0f;
	float posz             = 0.0f;

	while (!WindowShouldClose()) {
		if (IsKeyPressed(KEY_R))
			toAutoRotate = !toAutoRotate;
		if (IsKeyPressed(KEY_Q))
			break;
		if (IsKeyPressed(KEY_I))
			toDisplayDetails = !toDisplayDetails;

		// Zoom
		// -------------------------------------------------------------
		float zoomFactor = GetMouseWheelMove();
		if (posz - zoomFactor > -15 && posz - zoomFactor < 10)
			posz -= zoomFactor;

		// Rotation
		// -------------------------------------------------------------
		float dt = GetFrameTime();

		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			Vector2 mouseDelta = GetMouseDelta();
			roty += 0.2 * mouseDelta.x;
			rotx -= 0.2 * mouseDelta.y;
		} else {
			if (toAutoRotate)
				roty -= 3 * dt;
		}

		BeginDrawing();
		{
			ClearBackground(BG_COL);

			BeginMode3D(camera);
			{
				// Draw Sphere
				// ---------------------------------------------
				rlTranslatef(0, 0, posz);
				rlRotatef(rotx, 1, 0, 0);
				rlRotatef(roty, 0, 1, 0);
				for (int i = 0; i < detail * detail; i++)
					DrawPoint3D(sp_points[i], SPHERE_COL);
			}
			EndMode3D();

			// Draw Details
			// -----------------------------------------------------
			if (toDisplayDetails) {
				DrawFPS(5, 5);
				DrawText(INFO_TEXT, 5, 30, INFO_SIZE, INFO_FG);
			}
		}
		EndDrawing();
	}

	CloseWindow();
	return 0;
}
