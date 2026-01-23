#include "raylib.h"
#include "raymath.h"
#include <algorithm>
#include <string>


struct CanvasTransform
{
	float scale;
	Vector2 offset;
};

constexpr int screenWidth { 480 };
constexpr int screenHeight { 360 };

CanvasTransform CalculateCanvasTransform()
{
	const float windowWidth { static_cast<float>(GetScreenWidth()) };
	const float windowHeight { static_cast<float>(GetScreenHeight()) };

	const float scale { std::min(
		windowWidth / static_cast<float>(screenWidth),
		windowHeight / static_cast<float>(screenHeight)
	) };

	Vector2 offset { (windowWidth - static_cast<float>(screenWidth) * scale) * 0.5f,
	(windowHeight - static_cast<float>(screenHeight) * scale) * 0.5f };

	return { scale, offset };
}

int main()
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(screenWidth, screenHeight, "Breakout");
	SetWindowMinSize(screenWidth, screenHeight);
	SetTargetFPS(60);

	Camera2D camera { 0 };
	camera.target = { 0,0 };

	Color backgroundColour { 32, 32, 32, 0 };

	Texture2D paddle { LoadTexture("../assets/image/paddle.png") };
	Vector2 paddlePosition {
		(static_cast<float>(screenWidth) / 2.0f) - (paddle.width / 2),
		screenHeight - paddle.height - 15
	};

	while (!WindowShouldClose())
	{
		CanvasTransform canvasTransform { CalculateCanvasTransform() };
		camera.zoom = canvasTransform.scale;
		camera.offset = canvasTransform.offset;

		std::string paddleXValue { "Paddle X: " };

		const float deltaTime { GetFrameTime() };


		constexpr float paddleSpeed { 500.0f };
		Vector2 targetPaddlePosition { paddlePosition };

		if (IsKeyDown(KEY_A) || IsKeyPressed(KEY_LEFT))
		{
			targetPaddlePosition.x -= paddleSpeed * deltaTime;
		}

		if (IsKeyDown(KEY_D) || IsKeyPressed(KEY_RIGHT))
		{
			targetPaddlePosition.x += paddleSpeed * deltaTime;
		}

		targetPaddlePosition.x = std::clamp(targetPaddlePosition.x, 0.0f, static_cast<float>(screenWidth) - static_cast<float>(paddle.width));
		paddlePosition.x = targetPaddlePosition.x;

		paddleXValue.append(std::to_string(paddlePosition.x));

		// Draw
		BeginDrawing();
		ClearBackground(backgroundColour);
			BeginMode2D(camera);
				DrawText(paddleXValue.c_str() , 5, 5, 10, GREEN);

				DrawTexture(paddle, paddlePosition.x, paddlePosition.y, WHITE);
			EndMode2D();
		EndDrawing();
	}

	CloseWindow();
}

