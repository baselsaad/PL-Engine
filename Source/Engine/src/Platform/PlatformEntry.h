#pragma once
#include "Core/Window.h"

namespace PAL
{
	struct EngineArgs
	{
		int ArgumentsCount;
		char** RawArgumentStrings;

		std::string AppName; //Debug
		WindowData EngineWindowData;
	};

	int ApplicationMain(int argc, char** args);
}