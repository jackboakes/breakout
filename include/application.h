#pragma once

#include <vector>
#include "layer.h"
#include <memory>
#include <type_traits>

class Application {
private:
	std::vector<std::unique_ptr<Layer>> m_layerStack;

	Application();
	~Application();

	void ProcessInput();
	void Update(float deltaTime);
	void Draw();
public:
	static Application& Instance();
	void Run();

	template<typename TLayer>
	requires(std::is_base_of_v<Layer, TLayer>)
	void PushLayer()
	{
		m_layerStack.push_back(std::make_unique<TLayer>());
	}

	void TransitionLayer(std::unique_ptr<Layer> toLayer)
	{
		// TODO:: Not sure how to handle this yet
	}
};