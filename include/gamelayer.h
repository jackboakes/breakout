#pragma once
#include "layer.h"
#include "raylib.h"
#include "entity.h"
#include <vector>
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
	Sound m_SoundLevelComplete;
	Sound m_SoundGameOver;

	void ResetGame();

public:
	GameLayer();
	~GameLayer() override;

	bool ProcessInput() override;
	void Update(float deltaTime) override;
	void Draw() override;
};