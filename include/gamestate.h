#pragma once
#include "raylib.h"
#include "entity.h"
#include <vector>

enum class GameMode
{
	PAUSED,
	PLAYING,
	LEVEL_CLEAR,
	GAME_OVER
};

struct GameState
{
	Camera2D m_Camera2D { 0 };
	std::vector<Entity> m_Entities;

	GameMode m_GameMode { GameMode::PAUSED };

	int m_Score { 0 };
	int m_HighScore { 0 };

	int m_currentBlocksPerRow { 7 };
	static constexpr int m_MaxBlocksPerRow { 15 };
	static constexpr int m_BlockPadding { 2 };
	static constexpr float m_BlockStartOffset { 30.0f };
	static constexpr int m_NumBlockRows { 4 };

	int m_BlockWidth { 0 };
	int m_BlockHeight { 0 };
};