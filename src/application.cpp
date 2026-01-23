#include "application.h"
#include "globals.h"

Application::Application()
{
	// Window
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

	InitWindow(GameResolution::width * 2, GameResolution::height * 2, "Breakout");
	SetWindowState(FLAG_WINDOW_MAXIMIZED);
	SetWindowMinSize(GameResolution::width, GameResolution::height);
	SetTargetFPS(60);
}

Application::~Application()
{
	m_layerStack.clear();
	CloseWindow();
}

Application& Application::Instance()
{
	static Application instance;
	return instance;
}

void Application::Run()
{
	while (!WindowShouldClose())
	{
		ProcessInput();
		float deltaTime { GetFrameTime() };
		Update(deltaTime);
		Draw();
	}
}

void Application::ProcessInput()
{
	for (auto iter { m_layerStack.rbegin() }; iter != m_layerStack.rend(); ++iter)
	{
		if ((*iter)->ProcessInput())
		{
			break;
		}
	}
}

void Application::Update(float deltaTime)
{
	for (const std::unique_ptr<Layer>& layer : m_layerStack)
	{
		layer->Update(deltaTime);
	}
}

void Application::Draw()
{
	BeginDrawing();
	for (const std::unique_ptr<Layer>& layer : m_layerStack)
	{
		layer->Draw();
	}
	EndDrawing();
}

