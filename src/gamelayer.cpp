#include "gamelayer.h"
#include "globals.h"
#include "raymath.h"
#include <algorithm>
#include <string>
#include <cmath>

GameLayer::GameLayer()
{
	m_Font = LoadFontEx("../assets/font/NES.ttf", 32, 0, 250);

	auto AddTexture { [&](const char* path) {
		Texture2D texture = LoadTexture(path);
		m_Textures[texture.id] = texture;
		return texture.id;
		} };

	Entity paddle;
	paddle.AddFlag(EntityFlags::MOVABLE | EntityFlags::VISIBLE | EntityFlags::COLLIDABLE);
	paddle.type =			EntityType::PLAYER;
	unsigned int paddleID	{ AddTexture("../assets/image/paddle.png") };
	paddle.textureID =		paddleID;
	paddle.width =			m_Textures[paddleID].width;
	paddle.height =			m_Textures[paddleID].height;
	paddle.position.x =		(GameResolution::f_Width / 2.0f) - (paddle.width / 2);
	paddle.position.y =		GameResolution::f_Height - paddle.height - 15;
	paddle.moveSpeed =		400.0f;
	m_Entities.push_back(paddle);

	Entity ball;
	ball.AddFlag(EntityFlags::MOVABLE | EntityFlags::VISIBLE | EntityFlags::COLLIDABLE);
	ball.type =				EntityType::BALL;
	unsigned int ballID		{ AddTexture("../assets/image/ball_default.png") };
	ball.textureID =		ballID;
	ball.width =			m_Textures[ballID].width;
	ball.height =			m_Textures[ballID].height;
	ball.position.x =		(GameResolution::f_Width / 2.0f) - (ball.width / 2);
	ball.position.y =		paddle.position.y - ball.height - 2;
	ball.moveSpeed =		250.0f;
	ball.direction =		{ -0.5f, -1.0f };
	Vector2Normalize(ball.direction);
	m_Entities.push_back(ball);

	
	// The order matters 0 = top 3 = bottom
	unsigned int blockTextureIds[4];
	blockTextureIds[0] = AddTexture("../assets/image/block_pink.png");
	blockTextureIds[1] = AddTexture("../assets/image/block_brown.png");
	blockTextureIds[2] = AddTexture("../assets/image/block_green.png"); 
	blockTextureIds[3] = AddTexture("../assets/image/block_blue.png"); 

	m_BlockWidth	= m_Textures[blockTextureIds[0]].width;
	m_BlockHeight	= m_Textures[blockTextureIds[0]].height;

	const int totalBlockWidth	{ (m_MaxBlocksPerRow * m_BlockWidth) + ((m_MaxBlocksPerRow - 1) * m_BlockPadding) };
	const float startX			{ (GameResolution::f_Width * 0.5f) - (static_cast<float>(totalBlockWidth) * 0.5f) };


	for (int i { 0 }; i < m_NumBlockRows; i++)
	{
		for (int j { 0 }; j < m_MaxBlocksPerRow; j++)
		{
			Entity block;
			block.type =			EntityType::BLOCK;
			block.textureID =		blockTextureIds[i];
			block.width =			m_BlockWidth;
			block.height =			m_BlockHeight;
			block.position.x =		startX + static_cast<float>(j * (m_BlockWidth + m_BlockPadding));
			block.position.y =		m_BlockStartOffset + i * (block.height + m_BlockPadding);
			block.targetPosition =	block.position;
			m_Entities.push_back(block);
		}
	}

	const int numBlocksToSkip { (m_MaxBlocksPerRow - m_currentBlocksPerRow) / 2 };
	int blockCounter { 0 };
	for (auto& block : m_Entities)
	{
		if (block.type != EntityType::BLOCK) continue;

		int column { blockCounter % m_MaxBlocksPerRow };

		if (column >= numBlocksToSkip && column < (numBlocksToSkip + m_currentBlocksPerRow))
		{
			block.AddFlag(EntityFlags::VISIBLE | EntityFlags::COLLIDABLE);
		}
		else
		{
			block.RemoveFlag(EntityFlags::VISIBLE | EntityFlags::COLLIDABLE);
		}

		blockCounter++;
	}
}

GameLayer::~GameLayer()
{
	for (auto& [id, texture] : m_Textures)
	{
		UnloadTexture(texture);
	}
	m_Textures.clear();

	UnloadFont(m_Font);
}

bool GameLayer::ProcessInput()
{
	bool inputProcessed { false };

	if (m_GameMode == GameMode::PAUSED)
	{
		if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_SPACE))
		{
			m_GameMode = GameMode::PLAYING;
			return true;
		}
		return false;
	}

	for (auto& entity : m_Entities)
	{
		if (entity.type == EntityType::PLAYER)
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

	switch (m_GameMode)
	{
	case GameMode::PAUSED:
	{

	}
	break;
	case GameMode::PLAYING:
	{
		//// Handle animating blocks
		for (auto& entity : m_Entities)
		{
			if (entity.HasFlag(EntityFlags::ANIMATING))
			{
				constexpr float lerpSpeed { 1.5f };
				entity.position.y = Lerp(entity.position.y, entity.targetPosition.y, lerpSpeed * deltaTime);

				if (fabs(entity.position.y - entity.targetPosition.y) <= 0.0f)
				{
					entity.position.y = entity.targetPosition.y;
					entity.RemoveFlag(EntityFlags::ANIMATING);
				}
			}
		}

		// Update movement
		for (auto& entity : m_Entities)
		{
			if (entity.HasFlag(EntityFlags::MOVABLE))
			{
				const float displacement { entity.moveSpeed * deltaTime };
				entity.position.x += entity.direction.x * displacement;
				entity.position.y += entity.direction.y * displacement;
			}

			if (entity.type == EntityType::PLAYER)
			{
				entity.position.x = std::clamp(entity.position.x, 0.0f, GameResolution::f_Width - static_cast<float>(entity.width));
			}
		}

		// Check Collisions with wall
		for (auto& ball : m_Entities)
		{
			if (ball.type != EntityType::BALL) continue;

			// Screen Bouncing
			if (ball.position.x <= 0 || ball.position.x + ball.width >= GameResolution::width)
			{
				ball.direction.x *= -1.0f;
				ball.position.x = std::clamp(ball.position.x, 0.0f, GameResolution::f_Width - ball.width);
			}

			if (ball.position.y <= 0)
			{
				ball.direction.y *= -1.0f;
				ball.position.y = std::max(0.0f, ball.position.y);
			}
		}

		// Check ball collision vs blocks
		for (auto& ball : m_Entities)
		{
			if (ball.type != EntityType::BALL) continue;

			Rectangle ballBounds { ball.GetCollider() };
			bool hasCollided { false };

			for (auto& block : m_Entities)
			{
				if (block.type != EntityType::BLOCK) continue;
				if (!block.HasFlag(EntityFlags::COLLIDABLE)) continue;

				Rectangle blockBounds { block.GetCollider() };

				if (CheckCollisionRecs(ballBounds, blockBounds))
				{
					m_Score += 50;
					block.RemoveFlag(COLLIDABLE);
					block.RemoveFlag(VISIBLE);

					// Ensure the ball flips direction only once in the case of the ball hitting inbetween two blocks
					if (!hasCollided)
					{
						// Calculate ball and block centers
						float ballCenterX	{ ball.position.x + ball.width * 0.5f };
						float ballCenterY	{ ball.position.y + ball.height * 0.5f };
						float blockCenterX	{ block.position.x + block.width * 0.5f };
						float blockCenterY	{ block.position.y + block.height * 0.5f };

						// Get direciton from block centre to the ball centre
						float deltaX { ballCenterX - blockCenterX };
						float deltaY { ballCenterY - blockCenterY };

						// Normalise by block dimensions to get aspect-ratio-independent comparison
						float normalizedX = deltaX / (block.width * 0.5f);
						float normalizedY = deltaY / (block.height * 0.5f);
						
						// The axis with larger absolute normalised value indicates which side was hit
						if (std::abs(normalizedX) > std::abs(normalizedY))
						{
							// Hit left or right side
							ball.direction.x *= -1.0f;  
						}
						else
						{
							// Hit top or bottom
							ball.direction.y *= -1.0f;  
						}
						
						hasCollided = true;
					}
				}
			}
		}

		// Check ball collision vs paddle
		for (auto& ball : m_Entities)
		{
			if (ball.type != EntityType::BALL) continue;

			Rectangle ballBounds { ball.GetCollider() };

			for (auto& paddle : m_Entities)
			{
				if (paddle.type != EntityType::PLAYER) continue;

				Rectangle paddleBounds { paddle.GetCollider() };

				if (CheckCollisionRecs(ballBounds, paddleBounds))
				{
					float paddleCenterX = paddle.position.x + paddle.width * 0.5f;
					float ballCenterX = ball.position.x + ball.width * 0.5f;

					// Dividing by (paddle.width * 0.5f) normalises the offset to the range ( -1, 1 )
					// The offset is ( 0, -1 ) when the ball hits the centre it will shoot it straight up,
					// ( -1 , -1 ) is the far left it will shoot the ball left
					// ( 1, -1 )  is the far right it will shoot the ball right
					float offsetX = (ballCenterX - paddleCenterX) / (paddle.width * 0.5f);

					// The y component always shoot us in the opposite y direction
					Vector2 newDirection = Vector2Normalize({ offsetX, -1.0f }) * Vector2Length(ball.direction);
					ball.direction = newDirection;

					ball.position.y = paddle.position.y - ball.height;
				}
			}
		}

		// Check for game over
		for (auto& ball : m_Entities)
		{
			if (ball.type != EntityType::BALL) continue;

			if (ball.position.y >= GameResolution::f_Height)
			{
				ball.RemoveFlag(EntityFlags::VISIBLE);
				m_GameMode = GameMode::GAME_OVER;
				break;
			}
		}

		// Check for level completion
		bool levelComplete { true };
		for (const auto& block : m_Entities)
		{
			if (block.type != EntityType::BLOCK) continue;
			
			if (block.HasFlag(EntityFlags::VISIBLE))
			{
				levelComplete = false;
			}
		}

		// TODO:: remove later debug cheat
		if (IsKeyPressed(KEY_R))
		{
			levelComplete = true;
		}

		if (levelComplete == true)
		{
			m_Score += 250;
			m_GameMode = GameMode::LEVEL_CLEAR;
		}
	}
	break;
	case GameMode::LEVEL_CLEAR:
	{
		// Respawn blocks
		m_currentBlocksPerRow += 2;
		m_currentBlocksPerRow = std::min(m_currentBlocksPerRow, m_MaxBlocksPerRow);


		const float formationHeight = m_BlockStartOffset + (m_NumBlockRows * m_BlockHeight) + ((m_NumBlockRows - 1) * m_BlockPadding);
		const float offscreenOffset = formationHeight + m_BlockHeight;

		// Move all blocks offscreen
		for (auto& block : m_Entities)
		{
			if (block.type != EntityType::BLOCK) continue;
			block.position.y = block.targetPosition.y - offscreenOffset;
			block.AddFlag(EntityFlags::ANIMATING);
		}

		// Update visibility flags based on level
		const int numBlocksToSkip { (m_MaxBlocksPerRow - m_currentBlocksPerRow) / 2 };
		int blockCounter { 0 };
		for (auto& block : m_Entities)
		{
			if (block.type != EntityType::BLOCK) continue;

			int column { blockCounter % m_MaxBlocksPerRow };

			if (column >= numBlocksToSkip && column < (numBlocksToSkip + m_currentBlocksPerRow))
			{
				block.AddFlag(EntityFlags::VISIBLE | EntityFlags::COLLIDABLE);
			}
			else
			{
				block.RemoveFlag(EntityFlags::VISIBLE | EntityFlags::COLLIDABLE | EntityFlags::ANIMATING);
			}

			blockCounter++;
		}
		m_GameMode = GameMode::PLAYING;
	}
	case GameMode::GAME_OVER:
	{

	}
	break;
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

	const std::string scoreText { std::to_string(m_Score) };
	const Vector2 scoreTextSize { MeasureTextEx(m_Font, scoreText.c_str(), 16, 2) };
	const float centreX { (GameResolution::f_Width - scoreTextSize.x) * 0.5f };
	const float centreY { (30.0f - scoreTextSize.y) * 0.5f };
	DrawTextEx(m_Font, scoreText.c_str(), { centreX, centreY }, 16, 2, WHITE);

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