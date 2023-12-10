#include "pch.h"
#include "Core/Engine.h"

int main()
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Debug::Init("Engine-Editor");
	PAL::Engine app;
	app.Run();
}