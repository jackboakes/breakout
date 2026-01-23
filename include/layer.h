#pragma once
#include "raylib.h"

class Layer {
private:

public:
	virtual ~Layer() = default;
	virtual bool ProcessInput() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Draw() = 0;
};