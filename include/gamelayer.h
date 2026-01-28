#pragma once
#include "layer.h"
#include "raylib.h"
#include "entity.h"
#include "gamestate.h"
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

struct CanvasTransform
{
	float scale;
	Vector2 offset;
};

class GameLayer : public Layer
{
private:
	GameState m_GameState { 0 };
	Camera2D m_Camera2D { 0 };
	std::unordered_map<unsigned int, Texture2D> m_Textures;

	const Color m_BackgroundColour { 32, 32, 32, 255 };
	CanvasTransform CalculateCanvasTransform() const;
	
	// sound 
	Sound m_SoundButton;
	Sound m_SoundBall;
	Sound m_SoundBrick;
	Sound m_SoundLevelComplete;
	Sound m_SoundGameOver;

	// ui
	Font m_Font;
	UIElement m_PanelGameOver;
	UIElement m_ButtonPlayAgain;
	void ResetGame();

	void UpdateEntities(float deltaTime);
	void HandleCollisions();
	void HandleWallCollisions();
	void HandleBlockCollisions();
	void HandlePaddleCollisions();
	void CheckGameRules();

public:
	GameLayer();
	~GameLayer() override;

	bool ProcessInput() override;
	void Update(float deltaTime) override;
	void Draw() override;
};