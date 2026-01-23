#include "gamelayer.h"
#include "globals.h"
#include <algorithm>

GameLayer::GameLayer()
{
	auto AddTexture = [&](const char* path) {
		Texture2D texture = LoadTexture(path);
		m_Textures[texture.id] = texture;
		return texture.id;
		};

	Entity paddle;
	unsigned int paddleID { AddTexture("../assets/image/paddle.png") };
	paddle.AddFlag(EntityFlags::PLAYER | EntityFlags::MOVABLE | EntityFlags::VISIBLE | EntityFlags::COLLIDABLE);
	paddle.textureID =		paddleID;
	paddle.width =			m_Textures[paddleID].width;
	paddle.height =			m_Textures[paddleID].height;
	paddle.position.x =		(GameResolution::f_Width / 2.0f) - (paddle.width / 2);
	paddle.position.y =		GameResolution::f_Height - paddle.height - 15;
	paddle.moveSpeed =		600.0f;
	m_Entities.push_back(paddle);

	Entity ball;
	unsigned int ballID { AddTexture("../assets/image/ball_default.png") };
	ball.AddFlag(EntityFlags::BALL | EntityFlags::MOVABLE | EntityFlags::VISIBLE | EntityFlags::COLLIDABLE);
	ball.textureID =		ballID;
	ball.width =			m_Textures[ballID].width;
	ball.height =			m_Textures[ballID].height;
	ball.position.x =		(GameResolution::f_Width / 2.0f) - (ball.width / 2);
	ball.position.y =		paddle.position.y - ball.height - 2;
	ball.moveSpeed =		100.0f;
	ball.direction =		{ -1.0f, -1.0f };
	m_Entities.push_back(ball);

	//constexpr int maxBlockPerRow { 7 };

	Entity block;
	unsigned int blockBlueID { AddTexture("../assets/image/block_blue.png") };
	block.AddFlag(EntityFlags::BLOCK | EntityFlags::VISIBLE | EntityFlags::COLLIDABLE);
	block.textureID = blockBlueID;
	block.width = m_Textures[blockBlueID].width;
	block.height = m_Textures[blockBlueID].height;
	block.position.x = (GameResolution::f_Width / 2.0f) - (ball.width / 2);
	block.position.y = block.height + 20;
	m_Entities.push_back(block);


	unsigned int blockBrownID { AddTexture("../assets/image/block_brown.png") };
	unsigned int blockGreenID { AddTexture("../assets/image/block_green.png") };
	unsigned int blockPinkID { AddTexture("../assets/image/block_pink.png") };


}

GameLayer::~GameLayer()
{
	for (auto& [id, texture] : m_Textures)
	{
		UnloadTexture(texture);
	}
	m_Textures.clear();
}

bool GameLayer::ProcessInput()
{
	bool inputProcessed { false };

	if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_SPACE))
	{
		m_Pause = true;
	}

	if (m_Pause == false)
	{
		return false;
	}

	for (auto& entity : m_Entities)
	{
		if (entity.HasFlag(EntityFlags::PLAYER))
		{
			entity.direction.x = 0.0f;

			if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
			{
				entity.direction.x -= 1.0f;
				inputProcessed = true;
			}

			if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
			{
				entity.direction.x += 1.0f;
				inputProcessed = true;
			}
		}
	}

	return inputProcessed;
}

void GameLayer::Update(float deltaTime)
{
	CanvasTransform canvasTransform { CalculateCanvasTransform() };
	m_Camera2D.zoom = canvasTransform.scale;
	m_Camera2D.offset = canvasTransform.offset;

	if (m_Pause == false)
	{
		return;
	}

	// Update movement
	for (auto& entity : m_Entities)
	{
		if (entity.HasFlag(EntityFlags::MOVABLE))
		{
			entity.position.x += entity.direction.x * entity.moveSpeed * deltaTime;
			entity.position.y += entity.direction.y * entity.moveSpeed * deltaTime;
		}

		if (entity.HasFlag(EntityFlags::PLAYER))
		{
			entity.position.x = std::clamp(entity.position.x, 0.0f, GameResolution::f_Width - static_cast<float>(entity.width));
		}
	}

	// Check Collisions with wall
	for (auto& ball : m_Entities)
	{
		if (!ball.HasFlag(EntityFlags::BALL)) continue;
	
			// Screen Bouncing
			if (ball.position.x <= 0 || ball.position.x + ball.width >= GameResolution::width)
			{
				ball.direction.x *= -1;
			}

			if (ball.position.y <= 0)
			{
				ball.direction.y *= -1;
			}

			// Reset when off screen
			if (ball.position.y > GameResolution::height)
			{
				ball.position.x = (GameResolution::f_Width / 2.0f) - (ball.width / 2);
				ball.position.y = (GameResolution::f_Height * 0.5f);
			}
		
	}

	// Check for collision with paddle or brick
	for (auto& ball : m_Entities)
	{
		if (!ball.HasFlag(EntityFlags::BALL)) continue;

		Rectangle ballBounds = ball.GetCollider();

		for (auto& other : m_Entities)
		{
			if (&ball == &other) continue;
			if (!other.HasFlag(EntityFlags::COLLIDABLE)) continue;

			Rectangle otherBounds = other.GetCollider();

			if (CheckCollisionRecs(ballBounds, otherBounds))
			{
				if (other.HasFlag(EntityFlags::BLOCK))
				{
					other.RemoveFlag(COLLIDABLE);
					other.RemoveFlag(VISIBLE);
				}

				if (other.HasFlag(EntityFlags::PLAYER))
				{
					ball.direction.y *= -1.0f;
				}


			}
		}
	}
}

void GameLayer::Draw()
{
	// Draw
	ClearBackground(m_BackgroundColour);
	BeginMode2D(m_Camera2D);

	for (const auto& entity : m_Entities)
	{
		if (entity.HasFlag(EntityFlags::VISIBLE))
		{
			auto search = m_Textures.find(entity.textureID);
			if (search != m_Textures.end())
			{
				DrawTexture(search->second, entity.position.x, entity.position.y, WHITE);
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