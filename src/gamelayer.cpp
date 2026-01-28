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
	m_GameState.m_Entities.push_back(paddle);

	Entity ball;
	ball.AddFlag(EntityFlags::MOVABLE | EntityFlags::VISIBLE | EntityFlags::COLLIDABLE);
	ball.type =				EntityType::BALL;
	unsigned int ballID		{ AddTexture("../assets/image/ball_default.png") };
	ball.textureID =		ballID;
	ball.width =			m_Textures[ballID].width;
	ball.height =			m_Textures[ballID].height;
	ball.position.x =		(GameResolution::f_Width / 2.0f) - (ball.width / 2);
	ball.position.y =		paddle.position.y - ball.height - 2;
	ball.moveSpeed =		300.0f;
	ball.direction =		{ -0.5f, -1.0f };
	Vector2Normalize(ball.direction);
	m_GameState.m_Entities.push_back(ball);

	
	// The order matters 0 = top 3 = bottom
	unsigned int blockTextureIds[4];
	blockTextureIds[0] = AddTexture("../assets/image/block_pink.png");
	blockTextureIds[1] = AddTexture("../assets/image/block_brown.png");
	blockTextureIds[2] = AddTexture("../assets/image/block_green.png"); 
	blockTextureIds[3] = AddTexture("../assets/image/block_blue.png"); 

	m_GameState.m_BlockWidth	= m_Textures[blockTextureIds[0]].width;
	m_GameState.m_BlockHeight	= m_Textures[blockTextureIds[0]].height;

	const int totalBlockWidth	{ (m_GameState.m_MaxBlocksPerRow * m_GameState.m_BlockWidth) + ((m_GameState.m_MaxBlocksPerRow - 1) * m_GameState.m_BlockPadding) };
	const float startX			{ (GameResolution::f_Width * 0.5f) - (static_cast<float>(totalBlockWidth) * 0.5f) };


	for (int i { 0 }; i < m_GameState.m_NumBlockRows; i++)
	{
		for (int j { 0 }; j < m_GameState.m_MaxBlocksPerRow; j++)
		{
			Entity block;
			block.type =			EntityType::BLOCK;
			block.textureID =		blockTextureIds[i];
			block.width = m_GameState.m_BlockWidth;
			block.height = m_GameState.m_BlockHeight;
			block.position.x =		startX + static_cast<float>(j * (m_GameState.m_BlockWidth + m_GameState.m_BlockPadding));
			block.position.y = m_GameState.m_BlockStartOffset + i * (block.height + m_GameState.m_BlockPadding);
			block.targetPosition =	block.position;
			m_GameState.m_Entities.push_back(block);
		}
	}

	const int numBlocksToSkip { (m_GameState.m_MaxBlocksPerRow - m_GameState.m_currentBlocksPerRow) / 2 };
	int blockCounter { 0 };
	for (auto& block : m_GameState.m_Entities)
	{
		if (block.type != EntityType::BLOCK) continue;

		int column { blockCounter % m_GameState.m_MaxBlocksPerRow };

		if (column >= numBlocksToSkip && column < (numBlocksToSkip + m_GameState.m_currentBlocksPerRow))
		{
			block.AddFlag(EntityFlags::VISIBLE | EntityFlags::COLLIDABLE);
		}
		else
		{
			block.RemoveFlag(EntityFlags::VISIBLE | EntityFlags::COLLIDABLE);
		}

		blockCounter++;
	}


	// UI
	unsigned int gameOverPanelID	{ AddTexture("../assets/image/game_over_panel.png") };
	unsigned int buttonNormalID		{ AddTexture("../assets/image/button_play_again.png") };
	unsigned int buttonPressedID	{ AddTexture("../assets/image/button_pressed_play_again.png") };

	// Calculate total height to centre both panel and button
	const float panelHeight		{ static_cast<float>(m_Textures[gameOverPanelID].height) };
	const float buttonHeight	{ static_cast<float>(m_Textures[buttonNormalID].height) };
	const float spacing			{ 6.0f };
	const float totalHeight		{ panelHeight + spacing + buttonHeight };
	const float startY			{ (GameResolution::f_Height - totalHeight) * 0.5f };

	m_PanelGameOver.textureID = gameOverPanelID;
	m_PanelGameOver.bounds = {
		(GameResolution::f_Width - m_Textures[gameOverPanelID].width) * 0.5f,
		startY,
		static_cast<float>(m_Textures[gameOverPanelID].width),
		panelHeight
	};

	m_ButtonPlayAgain.textureID = buttonNormalID;
	m_ButtonPlayAgain.pressedTextureID = buttonPressedID;
	m_ButtonPlayAgain.bounds = {
		(GameResolution::f_Width - m_Textures[buttonNormalID].width) * 0.5f,
		startY + panelHeight + spacing,
		static_cast<float>(m_Textures[buttonNormalID].width),
		buttonHeight
	};


	// Sound
	InitAudioDevice();
	m_SoundButton =			LoadSound("../assets/sound/button_pressed.wav");
	m_SoundBrick =			LoadSound("../assets/sound/brick.wav");
	m_SoundBall =			LoadSound("../assets/sound/ball.wav");
	m_SoundLevelComplete =	LoadSound("../assets/sound/level_complete.wav");
	m_SoundGameOver =		LoadSound("../assets/sound/game_over.wav");
}

GameLayer::~GameLayer()
{
	for (auto& [id, texture] : m_Textures)
	{
		UnloadTexture(texture);
	}
	m_Textures.clear();

	UnloadFont(m_Font);
	UnloadSound(m_SoundButton);
	UnloadSound(m_SoundBall);
	UnloadSound(m_SoundBrick);
	UnloadSound(m_SoundLevelComplete);
	UnloadSound(m_SoundGameOver);

	CloseAudioDevice();
}

// Set up the game for the next game after the player clicks play again
void GameLayer::ResetGame()
{
	// Reset score
	m_GameState.m_Score = 0;

	// Reset blocks per row to initial value
	m_GameState.m_currentBlocksPerRow = 7;

	// Reset paddle positions
	for (auto& paddle : m_GameState.m_Entities)
	{
		if (paddle.type == EntityType::PLAYER)
		{
			// Reset paddle to center
			paddle.position.x = (GameResolution::f_Width / 2.0f) - (paddle.width / 2);
			paddle.position.y = GameResolution::f_Height - paddle.height - 15;
			paddle.direction = { 0.0f, 0.0f };
		}
	}

	// Reset ball position relative to the paddle
	for (auto& ball : m_GameState.m_Entities)
	{
		if (ball.type != EntityType::BALL) continue;
		
		// Find paddle to position ball relative to it
		for (const auto& paddle : m_GameState.m_Entities)
		{
			if (paddle.type != EntityType::PLAYER) continue;
			
			ball.position.x = (GameResolution::f_Width / 2.0f) - (ball.width / 2);
			ball.position.y = paddle.position.y - ball.height - 2;
			break;
		}
		ball.direction = { -0.5f, -1.0f };
		ball.AddFlag(EntityFlags::VISIBLE);
	}

	// Reset block visibility based on m_currentBlocksPerRow
	const int numBlocksToSkip { (m_GameState.m_MaxBlocksPerRow - m_GameState.m_currentBlocksPerRow) / 2 };
	int blockCounter { 0 };
	for (auto& block : m_GameState.m_Entities)
	{
		if (block.type != EntityType::BLOCK) continue;

		int column { blockCounter % m_GameState.m_MaxBlocksPerRow };

		// Reset position to target (in case of animation state)
		block.position = block.targetPosition;
		block.RemoveFlag(EntityFlags::ANIMATING);

		if (column >= numBlocksToSkip && column < (numBlocksToSkip + m_GameState.m_currentBlocksPerRow))
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

bool GameLayer::ProcessInput()
{
	bool inputProcessed { false };

	if (m_GameState.m_GameMode == GameMode::PAUSED)
	{
		if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER))
		{
			m_GameState.m_GameMode = GameMode::PLAYING;
			return true;
		}
		return false;
	}

	for (auto& entity : m_GameState.m_Entities)
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

	if (m_GameState.m_GameMode == GameMode::GAME_OVER)
	{
		Vector2 mousePos { GetMousePosition() };
		Vector2 gameMousePos { GetScreenToWorld2D(mousePos, m_Camera2D) };

		m_ButtonPlayAgain.isPressed = false;

		if (CheckCollisionPointRec(gameMousePos, m_ButtonPlayAgain.bounds))
		{
			if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
			{
				m_ButtonPlayAgain.isPressed = true;
			}

			if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
			{
				Audio::PlaySoundRandomisedPitch(m_SoundButton);
				ResetGame();
				m_GameState.m_GameMode = GameMode::PAUSED;
			}
		}

		if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER))
		{
			ResetGame();
			m_GameState.m_GameMode = GameMode::PAUSED;
		}
	}

	return inputProcessed;
}

void GameLayer::Update(float deltaTime)
{
	CanvasTransform canvasTransform { CalculateCanvasTransform() };
	m_Camera2D.zoom = canvasTransform.scale;
	m_Camera2D.offset = canvasTransform.offset;

	switch (m_GameState.m_GameMode)
	{
	case GameMode::PAUSED:
	{

	}
	break;
	case GameMode::PLAYING:
	{
		UpdateEntities(deltaTime);
		HandleCollisions();
		CheckGameRules();
	}
	break;
	case GameMode::LEVEL_CLEAR:
	{
		// Respawn blocks
		m_GameState.m_currentBlocksPerRow += 2;
		m_GameState.m_currentBlocksPerRow = std::min(m_GameState.m_currentBlocksPerRow, m_GameState.m_MaxBlocksPerRow);


		const float totalBlockHeight { m_GameState.m_BlockStartOffset + 
			(m_GameState.m_NumBlockRows * m_GameState.m_BlockHeight) + 
			((m_GameState.m_NumBlockRows - 1) * m_GameState.m_BlockPadding) };
		const float offscreenOffset { totalBlockHeight + m_GameState.m_BlockHeight };

		// Move all blocks offscreen
		for (auto& block : m_GameState.m_Entities)
		{
			if (block.type != EntityType::BLOCK) continue;
			block.position.y = block.targetPosition.y - offscreenOffset;
			block.AddFlag(EntityFlags::ANIMATING);
		}

		// Update visibility flags based on level
		const int numBlocksToSkip { (m_GameState.m_MaxBlocksPerRow - m_GameState.m_currentBlocksPerRow) / 2 };
		int blockCounter { 0 };
		for (auto& block : m_GameState.m_Entities)
		{
			if (block.type != EntityType::BLOCK) continue;

			int column { blockCounter % m_GameState.m_MaxBlocksPerRow };

			if (column >= numBlocksToSkip && column < (numBlocksToSkip + m_GameState.m_currentBlocksPerRow))
			{
				block.AddFlag(EntityFlags::VISIBLE | EntityFlags::COLLIDABLE);
			}
			else
			{
				block.RemoveFlag(EntityFlags::VISIBLE | EntityFlags::COLLIDABLE | EntityFlags::ANIMATING);
			}

			blockCounter++;
		}
		m_GameState.m_GameMode = GameMode::PLAYING;
	} 
	break;
	case GameMode::GAME_OVER:
	{

	}
	break;
	}	
}

void GameLayer::Draw()
{
	// Darker gray than the background
	ClearBackground({ 28, 28, 28 });
	BeginMode2D(m_Camera2D);

	DrawRectangle(0, 0, GameResolution::width, GameResolution::height, m_BackgroundColour);

	for (const auto& entity : m_GameState.m_Entities)
	{
		if (entity.HasFlag(EntityFlags::VISIBLE))
		{
			const Texture2D& entityTexture { m_Textures.at(entity.textureID) };
			DrawTexture(entityTexture, entity.position.x, entity.position.y, WHITE);
		}
	}

	const float centreX { GameResolution::f_Width * 0.5f };
	const float centreY { GameResolution::f_Height * 0.5f };

	const std::string scoreText { std::to_string(m_GameState.m_Score) };
	const Vector2 scoreTextSize { MeasureTextEx(m_Font, scoreText.c_str(), 16, 2) };
	const float centreTextX { centreX - (scoreTextSize.x * 0.5f) };
	const float centreTextY { (m_GameState.m_BlockStartOffset - scoreTextSize.y) * 0.5f };
	DrawTextEx(m_Font, scoreText.c_str(), { centreTextX, centreTextY }, 16, 2, WHITE);

	if (m_GameState.m_GameMode == GameMode::GAME_OVER)
	{
		// Dim the background
		DrawRectangle(0, 0, GameResolution::width, GameResolution::height, Fade(BLACK, 0.25f));

		const Texture2D& panelTexture { m_Textures.at(m_PanelGameOver.textureID) };
		DrawTexture(panelTexture, m_PanelGameOver.bounds.x, m_PanelGameOver.bounds.y, WHITE);


		// Draw score and high score on panel
		constexpr float fontSize { 16 };
		constexpr float fontSpacing { 2 };
		constexpr float labelValueSpacing { 4.0f };  // vertical gap between label and value
		constexpr float columnSpacing { 40.0f };      // horizontal gap between score and high columns

		// Score column
		const std::string scoreLabelText { "score" };
		const std::string scoreValueText { std::to_string(m_GameState.m_Score) };
		const Vector2 scoreLabelSize { MeasureTextEx(m_Font, scoreLabelText.c_str(), fontSize, fontSpacing) };
		const Vector2 scoreValueSize { MeasureTextEx(m_Font, scoreValueText.c_str(), fontSize, fontSpacing) };
		const float scoreColumnWidth { std::max(scoreLabelSize.x, scoreValueSize.x) };

		// High score column
		const std::string highLabelText { "high" };
		const std::string highValueText { std::to_string(m_GameState.m_HighScore) };
		const Vector2 highLabelSize { MeasureTextEx(m_Font, highLabelText.c_str(), fontSize, fontSpacing) };
		const Vector2 highValueSize { MeasureTextEx(m_Font, highValueText.c_str(), fontSize, fontSpacing) };
		const float highColumnWidth { std::max(highLabelSize.x, highValueSize.x) };

		// Calculate total dimensions for centering
		const float totalWidth { scoreColumnWidth + columnSpacing + highColumnWidth };
		const float rowHeight { fontSize + labelValueSpacing + fontSize };

		// Center the entire block in the panel
		const float startX { m_PanelGameOver.bounds.x + (m_PanelGameOver.bounds.width - totalWidth) * 0.5f };
		const float startY { m_PanelGameOver.bounds.y + (m_PanelGameOver.bounds.height - rowHeight) * 0.5f };

		// Draw score column (centred within its column)
		const float scoreLabelX { startX + (scoreColumnWidth - scoreLabelSize.x) * 0.5f };
		const float scoreValueX { startX + (scoreColumnWidth - scoreValueSize.x) * 0.5f };
		DrawTextEx(m_Font, scoreLabelText.c_str(), { scoreLabelX, startY }, fontSize, fontSpacing, WHITE);
		DrawTextEx(m_Font, scoreValueText.c_str(), { scoreValueX, startY + fontSize + labelValueSpacing }, fontSize, fontSpacing, WHITE);

		// Draw high score column (centred within its column)
		const float highColumnStartX { startX + scoreColumnWidth + columnSpacing };
		const float highLabelX { highColumnStartX + (highColumnWidth - highLabelSize.x) * 0.5f };
		const float highValueX { highColumnStartX + (highColumnWidth - highValueSize.x) * 0.5f };
		DrawTextEx(m_Font, highLabelText.c_str(), { highLabelX, startY }, fontSize, fontSpacing, WHITE);
		DrawTextEx(m_Font, highValueText.c_str(), { highValueX, startY + fontSize + labelValueSpacing }, fontSize, fontSpacing, WHITE);

		// Draw Game Over text
		const std::string gameOverText { "game over" };
		const Vector2 gameOverTextSize { MeasureTextEx(m_Font, gameOverText.c_str(), 22, 2) };
		const float gameOverTextX { m_PanelGameOver.bounds.x + (m_PanelGameOver.bounds.width - gameOverTextSize.x) * 0.5f };
		const float gameOverTextY { m_PanelGameOver.bounds.y + 15 };
		DrawTextEx(m_Font, gameOverText.c_str(), { gameOverTextX, gameOverTextY }, 22, 2, WHITE);

		// Button texture
		unsigned int buttonTextureID { m_ButtonPlayAgain.isPressed ?
				m_ButtonPlayAgain.pressedTextureID : m_ButtonPlayAgain.textureID };
		const Texture2D& buttonTexture { m_Textures.at(buttonTextureID) };
		DrawTexture(buttonTexture, m_ButtonPlayAgain.bounds.x, m_ButtonPlayAgain.bounds.y, WHITE);
	}

	EndMode2D();
}

void GameLayer::UpdateEntities(float deltaTime)
{
	// Handle animating blocks
	for (auto& entity : m_GameState.m_Entities)
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
	for (auto& entity : m_GameState.m_Entities)
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
}

void GameLayer::HandleCollisions()
{
	HandleWallCollisions();
	HandleBlockCollisions();
	HandlePaddleCollisions();
}
void GameLayer::HandleWallCollisions()
{
	// Check Collisions with wall
	bool wallSoundTrigger { false };
	for (auto& ball : m_GameState.m_Entities)
	{
		if (ball.type != EntityType::BALL) continue;

		// Screen Bouncing
		if (ball.position.x <= 0 || ball.position.x + ball.width >= GameResolution::width)
		{
			ball.direction.x *= -1.0f;
			ball.position.x = std::clamp(ball.position.x, 0.0f, GameResolution::f_Width - ball.width);
			wallSoundTrigger = true;
		}

		if (ball.position.y <= 0)
		{
			ball.direction.y *= -1.0f;
			ball.position.y = std::max(0.0f, ball.position.y);
			wallSoundTrigger = true;
		}
	}
	if (wallSoundTrigger)
	{
		if (!IsSoundPlaying(m_SoundBall))
		{
			Audio::PlaySoundRandomisedPitch(m_SoundBall);
		}
	}
}
void GameLayer::HandleBlockCollisions()
{
	// Check ball collision vs blocks
	bool brickSoundTrigger { false };
	for (auto& ball : m_GameState.m_Entities)
	{
		if (ball.type != EntityType::BALL) continue;

		Rectangle ballBounds { ball.GetCollider() };
		bool hasCollided { false };

		for (auto& block : m_GameState.m_Entities)
		{
			if (block.type != EntityType::BLOCK) continue;
			if (!block.HasFlag(EntityFlags::COLLIDABLE)) continue;

			Rectangle blockBounds { block.GetCollider() };

			if (CheckCollisionRecs(ballBounds, blockBounds))
			{
				brickSoundTrigger = true;
				m_GameState.m_Score += 50;
				block.RemoveFlag(COLLIDABLE);
				block.RemoveFlag(VISIBLE);

				// Ensure the ball flips direction only once in the case of the ball hitting inbetween two blocks
				if (!hasCollided)
				{
					// Calculate ball and block centers
					float ballCenterX { ball.position.x + ball.width * 0.5f };
					float ballCenterY { ball.position.y + ball.height * 0.5f };
					float blockCenterX { block.position.x + block.width * 0.5f };
					float blockCenterY { block.position.y + block.height * 0.5f };

					// Get direciton from block centre to the ball centre
					float deltaX { ballCenterX - blockCenterX };
					float deltaY { ballCenterY - blockCenterY };

					// Normalise by block dimensions to get aspect-ratio-independent comparison
					float normalisedX { deltaX / (block.width * 0.5f) };
					float normalisedY { deltaY / (block.height * 0.5f) };

					// The component with the larger absolute normalised value indicates which side was hit
					if (std::abs(normalisedX) > std::abs(normalisedY))
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

	if (brickSoundTrigger)
	{
		if (!IsSoundPlaying(m_SoundBrick))
		{
			Audio::PlaySoundRandomisedPitch(m_SoundBrick);
		}
	}
}

void GameLayer::HandlePaddleCollisions()
{
	bool paddleSoundTrigger { false };
	for (auto& ball : m_GameState.m_Entities)
	{
		if (ball.type != EntityType::BALL) continue;

		Rectangle ballBounds { ball.GetCollider() };

		for (auto& paddle : m_GameState.m_Entities)
		{
			if (paddle.type != EntityType::PLAYER) continue;

			Rectangle paddleBounds { paddle.GetCollider() };

			if (CheckCollisionRecs(ballBounds, paddleBounds))
			{
				paddleSoundTrigger = true;
				float paddleCenterX { paddle.position.x + paddle.width * 0.5f };
				float ballCenterX { ball.position.x + ball.width * 0.5f };

				// Calculate collision centers
				float paddleCenterY { paddle.position.y + paddle.height * 0.5f };
				float ballCenterY { ball.position.y + ball.height * 0.5f };

				// Get direction from paddle centre to the ball centre
				float deltaX { ballCenterX - paddleCenterX };
				float deltaY { ballCenterY - paddleCenterY };

				// Normalise by paddle dimensions to get aspect-ratio-independent comparison
				float normalisedX { deltaX / (paddle.width * 0.5f) };
				float normalisedY { deltaY / (paddle.height * 0.5f) };

				// Scale to make the bounce flatter 
				float deflection { normalisedX * 1.5f };

				// The y component always shoot us in the opposite y direction
				Vector2 newDirection { Vector2Normalize({ deflection, -1.0f }) };
				ball.direction = newDirection;

				// If its a side hit snap x position to side to prevent overlap
				if (std::abs(normalisedX) >= std::abs(normalisedY))
				{
					if (deltaX < 0)
					{
						// Left side
						ball.position.x = paddle.position.x - ball.width;
					}
					else
					{
						// Right side
						ball.position.x = paddle.position.x + paddle.width;
					}

				}
			}
		}
	}

	if (paddleSoundTrigger)
	{
		if (!IsSoundPlaying(m_SoundBall))
		{
			Audio::PlaySoundRandomisedPitch(m_SoundBall);
		}
	}
}

void GameLayer::CheckGameRules()
{
	// Check for game over
	for (auto& ball : m_GameState.m_Entities)
	{
		if (ball.type != EntityType::BALL) continue;

		if (ball.position.y >= GameResolution::f_Height)
		{
			ball.RemoveFlag(EntityFlags::VISIBLE);
			Audio::PlaySoundRandomisedPitch(m_SoundGameOver);
			m_GameState.m_HighScore = std::max(m_GameState.m_Score, m_GameState.m_HighScore);
			m_GameState.m_GameMode = GameMode::GAME_OVER;
			break;
		}
	}

	// Check for level completion
	bool levelComplete { true };
	for (const auto& block : m_GameState.m_Entities)
	{
		if (block.type != EntityType::BLOCK) continue;

		if (block.HasFlag(EntityFlags::VISIBLE))
		{
			levelComplete = false;
		}
	}

	if (levelComplete)
	{
		m_GameState.m_Score += 250;
		Audio::PlaySoundRandomisedPitch(m_SoundLevelComplete);
		m_GameState.m_GameMode = GameMode::LEVEL_CLEAR;
	}
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