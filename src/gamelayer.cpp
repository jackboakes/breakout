#include "gamelayer.h"
#include "globals.h"
#include <algorithm>
#include <string>

GameLayer::GameLayer()
{
	Texture2D paddleTexture = LoadTexture("../assets/image/paddle.png");
	m_Textures.push_back(paddleTexture);

	Entity paddle;
	paddle.type =		EntityType::Paddle;
	paddle.textureID =	paddleTexture.id;
	paddle.width =		paddleTexture.width;
	paddle.height =		paddleTexture.height;
	paddle.position.x = (GameResolution::f_Width / 2.0f) - (paddle.width / 2);
	paddle.position.y =	GameResolution::f_Height - paddle.height - 15;
	paddle.moveSpeed =	600.0f;
	m_Entities.push_back(paddle);

	Texture2D ballTexture = LoadTexture("../assets/image/ball_default.png");
	m_Textures.push_back(ballTexture);

	Entity ball;
	ball.type = EntityType::Ball;
	ball.textureID = ballTexture.id;
	ball.width = ballTexture.width;
	ball.height = ballTexture.height;
	ball.position.x = (GameResolution::f_Width / 2.0f) - (ball.width / 2);
	ball.position.y = paddle.position.y - ball.height - 2;
	ball.moveSpeed = 100.0f;
	m_Entities.push_back(ball);

	Texture2D blockBlueTexture = LoadTexture("../assets/image/block_blue.png");
	m_Textures.push_back(blockBlueTexture);
	Texture2D blockBrownTexture = LoadTexture("../assets/image/block_brown.png");
	m_Textures.push_back(blockBrownTexture);
	Texture2D blockGreenTexture = LoadTexture("../assets/image/block_green.png");
	m_Textures.push_back(blockGreenTexture);
	Texture2D blockPinkTexture = LoadTexture("../assets/image/block_pink.png");
	m_Textures.push_back(blockPinkTexture);
}

GameLayer::~GameLayer()
{
	for (auto& texture : m_Textures)
	{
		UnloadTexture(texture);
	}
	m_Textures.clear();
}

bool GameLayer::ProcessInput()
{
	bool inputProcessed { false };

	float deltaTime { GetFrameTime() };

	for (auto& entity : m_Entities)
	{
		switch (entity.type)
		{
		case EntityType::Paddle:
		{
			Vector2 targetPaddlePosition = entity.position;

			if (IsKeyDown(KEY_A) || IsKeyPressed(KEY_LEFT))
			{
				targetPaddlePosition.x -= entity.moveSpeed * deltaTime;
				inputProcessed = true;
			}

			if (IsKeyDown(KEY_D) || IsKeyPressed(KEY_RIGHT))
			{
				targetPaddlePosition.x += entity.moveSpeed * deltaTime;
				inputProcessed = true;
			}

			targetPaddlePosition.x = std::clamp(targetPaddlePosition.x, 0.0f, GameResolution::f_Width - static_cast<float>(entity.width));
			entity.position.x = targetPaddlePosition.x;
		}
		break;
		}
	}

	return inputProcessed;
}

void GameLayer::Update(float deltaTime)
{
	CanvasTransform canvasTransform { CalculateCanvasTransform() };
	m_Camera2D.zoom = canvasTransform.scale;
	m_Camera2D.offset = canvasTransform.offset;
}

void GameLayer::Draw()
{
	// Draw
	ClearBackground(m_BackgroundColour);
	BeginMode2D(m_Camera2D);

		for (const auto& entity : m_Entities)
		{
			if (entity.isVisible)
			{
				for (const auto& texture : m_Textures)
				{
					if (entity.textureID == texture.id)
					{
						DrawTexture(texture, entity.position.x, entity.position.y, WHITE);
					}
				}
			}
		}

	EndMode2D();

}

CanvasTransform GameLayer::CalculateCanvasTransform() const
{
	const float windowWidth { static_cast<float>(GetScreenWidth()) };
	const float windowHeight { static_cast<float>(GetScreenHeight()) };

	const float scale { std::min(
		windowWidth / GameResolution::f_Width,
		windowHeight / GameResolution::f_Height
	) };

	Vector2 offset { (windowWidth - GameResolution::f_Width * scale) * 0.5f,
	(windowHeight - GameResolution::f_Height * scale) * 0.5f };

	return { scale, offset };
}