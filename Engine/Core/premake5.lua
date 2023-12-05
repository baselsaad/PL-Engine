project "Core"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin/Intermediate/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "src/pch.cpp"

	files
	{
		"src/**.h",
		"src/**.cpp",

		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",

		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",

		"vendor/spdlog/inlcude/spdlog/spdlog.h",
		"vendor/spdlog/inlcude/spdlog/fmt/ostr.h",
		"vendor/spdlog/inlcude/spdlog/sinks/stdout_color_sinks.h",
		
		"vendor/VulkanMemoryAllocator-3.0.1/include/**.h",
		"vendor/VulkanMemoryAllocator-3.0.1/include/**.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"src",
		"vendor/",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.stb_image}"
	}

	links 
	{ 
		"GLFW",
		"ImGui",
		"%{Library.Vulkan}",
		"%{Library.VulkanUtils}"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"GLEW_STATIC",
			"PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
	  	defines "DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
	    defines "RELEASE"
		runtime "Release"
		optimize "on"