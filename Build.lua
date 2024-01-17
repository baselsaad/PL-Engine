-- PL-Engine
workspace "PAL-Engine"
	architecture "x64"
	startproject "Editor"

	configurations
	{
		"Debug",
		"Release",
        "Shipping"
	}
	
	flags
	{
		"MultiProcessorCompile"
	}

    prebuildcommands
    {
        "call %{wks.location}Source/Engine/res/shaders/compile.bat",
    }

outputdir = "%{cfg.buildcfg}_%{cfg.system}"

VULKAN_SDK = os.getenv("VULKAN_SDK")
ProjectRoot = os.getcwd()

-- Include directories relative 
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Source/Engine/vendor/GLFW/include"
IncludeDir["glm"] =  "%{wks.location}/Source/Engine/vendor/glm-0.9.9.8/"
IncludeDir["stb_image"] = "%{wks.location}/Source/Engine/vendor/stb_image"
IncludeDir["imgui"] = "%{wks.location}/Source/Engine/vendor/imgui/"
IncludeDir["spdlog"] = "%{wks.location}/Source/Engine/vendor/spdlog/include"
IncludeDir["Instrumentation"] = "%{wks.location}/Source/Engine/vendor/Instrumentation/"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["ECS"] = "%{wks.location}/Source/Engine/vendor/entt/single_include"
IncludeDir["VulkanMemoryAllocator"] = "%{wks.location}/Source/Engine/vendor/VulkanMemoryAllocator-3.0.1/include"
IncludeDir["Optick"] = "%{wks.location}/Source/Engine/vendor/optick/src/"

Library = {}
Library["Vulkan"] = "%{VULKAN_SDK}/Lib/vulkan-1.lib"


-- Projects
group "Dependencies"
	include "Source/Engine/vendor/GLFW/Build-GLFW.lua"
    include "Source/Engine/vendor/imgui/Build-imgui.lua"
    include "Source/Engine/vendor/optick/Build-optick.lua"
group ""

include "Source/Engine/Build-Engine.lua"
include "Source/Editor/Build-Editor.lua"

newaction {
    trigger = "clean",
    description = "Remove all binaries and intermediate binaries, and vs files.",
    execute = function()

        print("Removing binaries")
        os.rmdir("./Binaries")

        print("Removing intermediate binaries")
        os.rmdir("./Intermediate")

        print("Removing project files")
        os.rmdir("./.vs")

        print("Removing Profiler files")
        os.rmdir("./Profiler")

        os.remove("**.sln")
        os.remove("**.vcxproj")
        os.remove("**.vcxproj.filters")
        os.remove("**.vcxproj.user")
        print("Done")
    end
}
