#include "pch.h"

#include "Platform/PlatformEntry.h"
#include "Core/Engine.h"
#include "Utilities/Timer.h"
#include "Utilities/Log.h"
#include "Core/MemoryTracker.h"

#if PLATFORM_WINDOWS 

namespace PAL
{
	// this function will be implemented in the Startup project (e.g. Sandbox, Editor, Runtime)
	extern EngineArgs ApplicationMain(int argc, char** argv);

	int Main(int argc, char** argv)
	{
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

		Debug::Init("Engine-Core");
		PAL::EngineArgs engineArgs = PAL::ApplicationMain(argc, argv);

		BEGIN_PROFILE_SESSION("Init Engine Session");
		PAL::Engine engine(engineArgs);
		END_PROFILE_SESSION();

		BEGIN_PROFILE_SESSION("EngineLoop Session");
		engine.Run();
		END_PROFILE_SESSION();

		BEGIN_PROFILE_SESSION("Engine Shutdown Session");
		engine.Stop();
		END_PROFILE_SESSION();

		

		return 0;
	}

}

#ifndef SHIPPING

int main(int argc, char** argv)
{
	return PAL::Main(argc, argv);
}

#else

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
{
	return PAL::Main(__argc, __argv);
}

#endif 

#endif 