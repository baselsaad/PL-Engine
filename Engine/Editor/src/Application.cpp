#include "pch.h"
#include "Core/Engine.h"
#include "Utilities/Timer.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Debug::Init("Engine-Editor");

	BEGIN_PROFILE_SESSION("Engine");
	PAL::Engine engine;
	engine.Run();
	engine.Stop();
	END_PROFILE_SESSION();
}