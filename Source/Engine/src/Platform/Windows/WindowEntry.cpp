#include "pch.h"
#include "Platform/PlatformEntry.h"

extern int PAL::ApplicationMain(int argc, char** args);

int main(int argc, char** args)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Debug::Init("Engine-Core");
	PAL::ApplicationMain(argc, args);
}