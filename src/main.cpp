#include "application.h"
#include "gamelayer.h"

int main()
{
	Application& application { Application::Instance() };
	application.PushLayer<GameLayer>();
	application.Run();
}

