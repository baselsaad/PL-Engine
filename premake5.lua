-- PL-Engine
workspace "PL-Engine"
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
        "call %{wks.location}Engine/Core/res/shaders/compile.bat",
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.platform}"

VULKAN_SDK = os.getenv("VULKAN_SDK")
EngineRoot = os.getcwd()

-- Include directories relative 
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Engine/Core/vendor/GLFW/include"
IncludeDir["glm"] =  "%{wks.location}/Engine/Core/vendor/glm-0.9.9.8/"
IncludeDir["stb_image"] = "%{wks.location}/Engine/Core/vendor/stb_image"
IncludeDir["imgui"] = "%{wks.location}/Engine/Core/vendor/imgui"
IncludeDir["spdlog"] = "%{wks.location}/Engine/Core/vendor/spdlog/include"
IncludeDir["Instrumentation"] = "%{wks.location}/Engine/Core/vendor/Instrumentation/"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["ECS"] = "%{wks.location}/Engine/Core/vendor/entt/single_include"

Library = {}
Library["Vulkan"] = "%{VULKAN_SDK}/Lib/vulkan-1.lib"
--Library["VulkanUtils"] = "%{VULKAN_SDK}/Lib/VkLayer_utils.lib"

-- Projects
group "Dependencies"
	include "Engine/Core/vendor/GLFW"
    include "Engine/Core/vendor/imgui"
group ""

include "Engine/Core"
include "Engine/Editor"

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
