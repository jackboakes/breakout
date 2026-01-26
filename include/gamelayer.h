#pragma once
#include "layer.h"
#include "raylib.h"
#include <vector>
#include <cstdint>
#include <unordered_map>

namespace Audio
{
	inline void PlaySoundRandomisedPitch(Sound& sound)
	{
		constexpr int lowerBound { 95 };
		constexpr int upperBound { 105 };
		float pitch { GetRandomValue(lowerBound, upperBound) / 100.0f };
		SetSoundPitch(sound, pitch);
		PlaySound(sound);
	}
};

enum class GameMode 
{
	PAUSED,
	PLAYING,
	LEVEL_CLEAR,
	GAME_OVER
};

struct UIElement
{
	Rectangle bounds { 0 };
	unsigned int textureID { 0 };
	unsigned int pressedTextureID { 0 };
	bool isPressed { false };
};

enum class EntityType 
{
	NONE = 0,
	PLAYER,
	BALL,
	BLOCK
};

enum EntityFlags : uint32_t 
{
	NONE		= 0,
	VISIBLE		= 1 << 0,
	MOVABLE		= 1 << 1,
	COLLIDABLE	= 1 << 2,
	ANIMATING	= 1 << 3
};

struct Entity
{
	EntityType type { EntityType::NONE };
	uint32_t flags { EntityFlags::NONE };

	// Bind the raylib texture ID to the entity
	unsigned int textureID { 0 };
	int width { 0 };
	int height { 0 };

	Vector2 position { 0.0f, 0.0f };
	Vector2 targetPosition { 0.0f, 0.0f };
	Vector2 direction { 0.0f, 0.0f };
	float moveSpeed { 0.0f };

	inline Rectangle GetCollider() const 
	{
		return Rectangle { position.x, position.y, (float)width, (float)height };
	}

	inline bool HasFlag(uint32_t flag) const 
	{
		return (flags & flag) != 0;
	}

	inline void AddFlag(uint32_t flag) 
	{
		flags |= flag;
	}

	inline void RemoveFlag(uint32_t flag) 
	{
		flags &= ~flag;
	}
};

struct CanvasTransform
{
	float scale;
	Vector2 offset;
};

class GameLayer : public Layer
{
private:
	Camera2D m_Camera2D { 0 };
	std::unordered_map<unsigned int, Texture2D> m_Textures;
	std::vector<Entity> m_Entities;

	const Color m_BackgroundColour { 32, 32, 32, 255 };
	CanvasTransform CalculateCanvasTransform() const;
	
	GameMode m_GameMode { GameMode::PAUSED };
	int m_currentBlocksPerRow { 7 };
	int m_Score { 0 };
	
	static constexpr int m_MaxBlocksPerRow		{ 15 };
	static constexpr int m_BlockPadding		{ 2 };
	static constexpr float m_BlockStartOffset	{ 30.0f };
	static constexpr int m_NumBlockRows		{ 4 };
	
	int m_BlockWidth { 0 };
	int m_BlockHeight { 0 };


	// ui
	// TODO:: placeholder;
	Font m_Font;

	UIElement m_PanelGameOver;
	UIElement m_ButtonPlayAgain;

	// sound 
	Sound m_SoundButton;
	Sound m_SoundBall;
	Sound m_SoundBrick;

	void ResetGame();

public:
	GameLayer();
	~GameLayer() override;

	bool ProcessInput() override;
	void Update(float deltaTime) override;
	void Draw() override;
};