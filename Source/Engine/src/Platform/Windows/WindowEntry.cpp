#include "pch.h"
#include "Platform/PlatformEntry.h"

// this function will be implemented in the setup project (e.g. Sandbox, Editor, Runtime)
extern int PAL::ApplicationMain(int argc, char** args);

int main(int argc, char** args)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Debug::Init("Engine-Core");
	PAL::ApplicationMain(argc, args);
}