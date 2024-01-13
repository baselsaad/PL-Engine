#include "pch.h"

#include "Platform/PlatformEntry.h"
#include "Editor.h"

namespace PAL
{
	EngineArgs ApplicationMain(int argc, char** argv)
	{
		EngineArgs engineArgs = {};
		engineArgs.ArgumentsCount = argc;
		engineArgs.RawArgumentStrings = argv;
		engineArgs.AppName = "Editor";

		WindowData window;
		window.Width = 1600;
		window.Height = 900;
		window.Title = "PAL Editor";
		window.Vsync = false;
		window.Mode = WindowMode::Windowed;

		engineArgs.EngineWindowData = window;
		engineArgs.CurrentApp = new Editor();

		return engineArgs;
	}
}