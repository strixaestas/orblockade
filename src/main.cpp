#include "raylib.h"
#include "raymath.h"

#include "GeodesicSphere.h"
#include "Skybox.h"

void InitializeScreen();

int main() {
	
	InitializeScreen();

	Camera camera = { };
	camera.fovy = 50.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	GeodesicSphere sphere = GeodesicSphere(4);
	Skybox skybox = Skybox("resources/textures/skybox.png");

	Mesh playerMesh = GenMeshCube(0.4f, 0.8f, 1.6f);
	Model player = LoadModelFromMesh(playerMesh);

	Vector3 upVector = { 0.0f, 1.0f, 0.0f };
	Vector3 forwardAxis = { 1.0f, 0.0f, 0.0f };

	// Distance from the center of the planet
	float altitude = 150.4f;

	// Forward movement parameters
	constexpr float FORWARD_SPEED = PI / 450.0f;
	constexpr float MAX_FORWARD_SPEED_MULTIPLIER = 2.0f;
	constexpr float FORWARD_SPEED_MULTIPLIER_INCREMENT = 0.1f;
	float currentForwardSpeed = FORWARD_SPEED;

	// Turn parameters
	constexpr float MAX_TURN_SPEED = PI / 60.0f;
	constexpr float TURN_SPEED_INCREMENT = MAX_TURN_SPEED / 8.0f;
	constexpr float TURBO_TURN_MULTIPLIER = 3.0f;
	constexpr float MAX_LEAN_ANGLE = PI / 12.0f;
	float currentTurnSpeed = 0.0f;

	// Jump parameters
	float jumpHeight = 0.0f;
	constexpr float JUMP_STRENGTH = 0.8f;
	constexpr float JUMP_SPEED = 0.05f;
	float jumpForce = 0.0f;
	bool jumping = false;

	while (!WindowShouldClose()) {
		
		///////////////////////////////////////////////////////////////
		// Turn the player "left" or "right" based on keyboard input //
		///////////////////////////////////////////////////////////////

		float turnMultiplier = 1.0f;

		// Turning is faster with LEFT SHIFT pressed
		if (IsKeyDown(KEY_LEFT_SHIFT)) {
			turnMultiplier = TURBO_TURN_MULTIPLIER;
		}

		// Update the turn speed based on key presses
		if (IsKeyDown(KEY_D) || IsKeyDown(KEY_A)) {

			// Increase the turn speed until it reaches maximum
			if (IsKeyDown(KEY_D)) {
				currentTurnSpeed -= TURN_SPEED_INCREMENT;
				if (currentTurnSpeed < -MAX_TURN_SPEED * turnMultiplier) {
					currentTurnSpeed += TURN_SPEED_INCREMENT;
				}
			}
			if (IsKeyDown(KEY_A)) {
				currentTurnSpeed += TURN_SPEED_INCREMENT;
				if (currentTurnSpeed > MAX_TURN_SPEED * turnMultiplier) {
					currentTurnSpeed -= TURN_SPEED_INCREMENT;
				}
			}
		}
		// If no turn key is pressed then gradually lower the turn speed down to zero
		else {
			if (currentTurnSpeed > 0.0f) {
				currentTurnSpeed -= TURN_SPEED_INCREMENT;
				if (currentTurnSpeed < 0.0f) {
					currentTurnSpeed = 0.0f;
				}
			}
			else if (currentTurnSpeed < 0.0f) {
				currentTurnSpeed += TURN_SPEED_INCREMENT;
				if (currentTurnSpeed > 0.0f) {
					currentTurnSpeed = 0.0f;
				}
			}
		}

		// Turn and lean the player
		if (currentTurnSpeed != 0.0f) {
			// Turn
			Matrix rotation = MatrixRotate(upVector, currentTurnSpeed);
			forwardAxis = Vector3Transform(forwardAxis, rotation);
			player.transform = MatrixMultiply(player.transform, rotation);
		}


		///////////////////////////////
		// Moving the player forward //
		///////////////////////////////

		float fs = currentForwardSpeed;

		// If KEY_DOWN is pressed then activate break
		if (IsKeyDown(KEY_S)) {
			fs /= MAX_FORWARD_SPEED_MULTIPLIER;
		}

		// If KEY_UP is pressed then activate boost
		if (IsKeyDown(KEY_W)) {
			fs *= MAX_FORWARD_SPEED_MULTIPLIER;
		}

		// If SPACE then jump
		if (IsKeyPressed(KEY_SPACE)) {
			if (!jumping) {
				jumping = true;
				jumpForce = JUMP_STRENGTH;
			}
		}

		// Jumping
		if (jumping) {
			jumpHeight += jumpForce;
			if (jumpHeight < 0.0f) {
				jumpHeight = 0.0f;
				jumping = false;
			}
			jumpForce -= JUMP_SPEED;
		}

		Matrix forwardRotation = MatrixRotate(forwardAxis, fs);
		upVector = Vector3Transform(upVector, forwardRotation);
		player.transform = MatrixMultiply(player.transform, forwardRotation);

		// Calculate player position
		Vector3 playerPosition = Vector3Scale(upVector, altitude + jumpHeight);


		///////////////////
		// Update camera //
		///////////////////

		// Find position behind the player
		Vector3 behindPlayerVector = Vector3RotateByAxisAngle(forwardAxis, upVector, PI / 2.0f);
		behindPlayerVector = Vector3RotateByAxisAngle(behindPlayerVector, forwardAxis, PI / 3.5f);
		behindPlayerVector = Vector3Scale(behindPlayerVector, 120.0f);

		// Lean camera in line with the player turning
		Vector3 leanAxis = Vector3RotateByAxisAngle(forwardAxis, upVector, -PI / 2.0f);
		float leanAngle = -currentTurnSpeed * MAX_LEAN_ANGLE / MAX_TURN_SPEED;
		//behindPlayerVector = Vector3RotateByAxisAngle(behindPlayerVector, leanAxis, leanAngle);

		// Establish final camera position
		camera.position = Vector3Add(playerPosition, behindPlayerVector);
		camera.target = Vector3Add(playerPosition, Vector3Scale(upVector, 7.0f));
		camera.up = upVector;


		// DEBUG
		// camera.position = { 600.0f, 0.0f, 0.0f };
		// camera.target = { 0.0f, 0.0f, 0.0f };
		// camera.up = { 0.0f, 1.0f, 0.0f };
		// camera.projection = CAMERA_ORTHOGRAPHIC;
		// camera.fovy = 360.0f;

		////////////////////		
		// Draw the scene //
		////////////////////
		BeginDrawing(); {
			ClearBackground(BLACK);
			BeginMode3D(camera); {
				// Draw the sky
				skybox.draw(&camera);
				
				// Draw the planet
				DrawModel(sphere.model, sphere.position, 149.9f, {0, 16, 32, 255});
				DrawModelWires(sphere.model, sphere.position, 150.0f, DARKGRAY);

				// Draw player
				DrawModelEx(player, playerPosition, leanAxis, leanAngle* RAD2DEG, Vector3One(), RED);
				DrawModelWiresEx(player, playerPosition, leanAxis, leanAngle * RAD2DEG, Vector3Scale(Vector3One(), 1.1f), YELLOW);
			} EndMode3D();
		} EndDrawing();

	}

	CloseWindow();

	return 0;
}


void InitializeScreen() {
	int monitor = GetCurrentMonitor();
	int width = GetMonitorWidth(monitor);
	int height = GetMonitorHeight(monitor);

	InitWindow(1920, 1080, "Spacetron!");
	ToggleFullscreen();
	SetTargetFPS(60);
}
