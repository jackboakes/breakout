#include "gamelayer.h"
#include "globals.h"
#include <algorithm>
#include "raymath.h"

GameLayer::GameLayer()
{
	auto AddTexture { [&](const char* path) {
		Texture2D texture = LoadTexture(path);
		m_Textures[texture.id] = texture;
		return texture.id;
		} };

	Entity paddle;
	paddle.AddFlag(EntityFlags::PLAYER | EntityFlags::MOVABLE | EntityFlags::VISIBLE | EntityFlags::COLLIDABLE);
	unsigned int paddleID	{ AddTexture("../assets/image/paddle.png") };
	paddle.textureID =		paddleID;
	paddle.width =			m_Textures[paddleID].width;
	paddle.height =			m_Textures[paddleID].height;
	paddle.position.x =		(GameResolution::f_Width / 2.0f) - (paddle.width / 2);
	paddle.position.y =		GameResolution::f_Height - paddle.height - 15;
	paddle.moveSpeed =		600.0f;
	m_Entities.push_back(paddle);

	Entity ball;
	ball.AddFlag(EntityFlags::BALL | EntityFlags::MOVABLE | EntityFlags::VISIBLE | EntityFlags::COLLIDABLE);
	unsigned int ballID		{ AddTexture("../assets/image/ball_default.png") };
	ball.textureID =		ballID;
	ball.width =			m_Textures[ballID].width;
	ball.height =			m_Textures[ballID].height;
	ball.position.x =		(GameResolution::f_Width / 2.0f) - (ball.width / 2);
	ball.position.y =		paddle.position.y - ball.height - 2;
	ball.moveSpeed =		100.0f;
	ball.direction =		{ -1.0f, -1.0f };
	Vector2Normalize(ball.direction);
	m_Entities.push_back(ball);

	
	// Load block textures
	// The order matters 0 = top 4 = bottom
	unsigned int blockTextureIds[4];
	blockTextureIds[0] = AddTexture("../assets/image/block_blue.png");
	blockTextureIds[1] = AddTexture("../assets/image/block_brown.png");
	blockTextureIds[2] = AddTexture("../assets/image/block_green.png");
	blockTextureIds[3] = AddTexture("../assets/image/block_pink.png");

	constexpr int maxBlockPerRow		{ 7 };
	constexpr int paddingBetweenBlock	{ 2 };
	
	const int blockWidth { m_Textures[blockTextureIds[0]].width };
	const int blockHeight { m_Textures[blockTextureIds[0]].height };

	const int totalBlockWidth { (maxBlockPerRow * blockWidth) + ((maxBlockPerRow - 1) * paddingBetweenBlock) };

	const float startX { (GameResolution::f_Width * 0.5f) - (static_cast<float>(totalBlockWidth) * 0.5f) };


	for (int i { 0 }; i < 4; i++)
	{
		for (int j { 0 }; j < maxBlockPerRow; j++)
		{
			Entity block;
			block.AddFlag(EntityFlags::BLOCK | EntityFlags::VISIBLE | EntityFlags::COLLIDABLE);
			block.textureID =		blockTextureIds[i];
			block.width =			m_Textures[blockTextureIds[i]].width;
			block.height =			m_Textures[blockTextureIds[i]].height;
			block.position.x =		startX + static_cast<float>(j * (blockWidth + paddingBetweenBlock));
			block.position.y =		20 + i * (block.height + paddingBetweenBlock);
			m_Entities.push_back(block);
		}
	}
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
			const float velocity { entity.moveSpeed * deltaTime };
			entity.position.x += entity.direction.x * velocity;
			entity.position.y += entity.direction.y * velocity;
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
				ball.direction.x *= -1.0f;
			}

			if (ball.position.y <= 0)
			{
				ball.direction.y *= -1.0f;
			}
	}

	// Check for collision with paddle or block
	for (auto& ball : m_Entities)
	{
		if (!ball.HasFlag(EntityFlags::BALL)) continue;

		Rectangle ballBounds { ball.GetCollider() };

		for (auto& other : m_Entities)
		{
			if (&ball == &other) continue;
			if (!other.HasFlag(EntityFlags::COLLIDABLE)) continue;

			Rectangle otherBounds { other.GetCollider() };

			if (CheckCollisionRecs(ballBounds, otherBounds))
			{
				if (other.HasFlag(EntityFlags::BLOCK))
				{
					other.RemoveFlag(COLLIDABLE);
					other.RemoveFlag(VISIBLE);
					// TODO:: this is a temp naive solution
					ball.direction.y *= -1.0f;
				}

				if (other.HasFlag(EntityFlags::PLAYER))
				{
					float paddleCenterX = other.position.x + other.width * 0.5f;
					float ballCenterX = ball.position.x + ball.width * 0.5f;

					// Dividing by (other.width * 0.5f) normalises the offset to the range ( -1, 1 )
					// The offset is ( 0, -1 ) when the ball hits the centre it will shoot it straight up,
					// ( -1 , -1 ) is the far left it will shoot the ball left
					// ( 1, -1 )  is the far right it will shoot the ball right
					float offsetX = (ballCenterX - paddleCenterX) / (other.width * 0.5f);
					
					// The y component always shoot us in the opposite y direction
					Vector2 newDirection = Vector2Normalize({ offsetX, -1.0f }) * Vector2Length(ball.direction);
					ball.direction = newDirection;
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
			auto search { m_Textures.find(entity.textureID) };
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