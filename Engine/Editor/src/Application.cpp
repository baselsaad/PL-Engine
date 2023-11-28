#include "pch.h"
#include "Core/Engine.h"

int main()
{
	Debug::Init("Engine-Editor");
	PL_Engine::Engine app;
	app.Run();
}