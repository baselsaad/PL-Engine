-- PL-Engine
workspace "PL-Engine"
	architecture "x64"
	startproject "Editor"

	configurations
	{
		"Debug",
		"Release"
	}
	
	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

VULKAN_SDK = os.getenv("VULKAN_SDK")

-- Include directories relative 
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Engine/Core/vendor/GLFW/include"
IncludeDir["glm"] =  "%{wks.location}/Engine/Core/vendor/glm"
IncludeDir["stb_image"] = "%{wks.location}/Engine/Core/vendor/stb_image"
IncludeDir["imgui"] = "%{wks.location}/Engine/Core/vendor/imgui"
IncludeDir["spdlog"] = "%{wks.location}/Engine/Core/vendor/spdlog/include"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"

Library = {}
Library["Vulkan"] = "%{VULKAN_SDK}/Lib/vulkan-1.lib"
Library["VulkanUtils"] = "%{VULKAN_SDK}/Lib/VkLayer_utils.lib"

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
        os.rmdir("./bin")
        print("Removing intermediate binaries")
        os.rmdir("./bin-int")
        print("Removing project files")
        os.rmdir("./.vs")
        os.remove("**.sln")
        os.remove("**.vcxproj")
        os.remove("**.vcxproj.filters")
        os.remove("**.vcxproj.user")
        print("Done")
    end
}
