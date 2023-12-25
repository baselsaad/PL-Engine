project "Core"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/Binaries/Engine/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/Intermediate/Engine/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "src/pch.cpp"

	files
	{
		"src/**.h",
		"src/**.cpp",

		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",

		"vendor/glm-0.9.9.8/glm/**.hpp",
		"vendor/glm-0.9.9.8/glm/**.inl",

		"vendor/spdlog/inlcude/spdlog/spdlog.h",
		"vendor/spdlog/inlcude/spdlog/fmt/ostr.h",
		"vendor/spdlog/inlcude/spdlog/sinks/stdout_color_sinks.h",
		
		"vendor/VulkanMemoryAllocator-3.0.1/include/**.h",
		"vendor/VulkanMemoryAllocator-3.0.1/include/**.cpp",

		"res/shaders/*.frag",
		"res/shaders/*.vert",

		"%{IncludeDir.Instrumentation}/*.h"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"ENGINE_ROOT=\"" .. EngineRoot .. "\""
	}

	includedirs
	{
		"%{wks.location}/Engine/Core/src",
		"%{wks.location}/Engine/Core/vendor",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.Instrumentation}",
		"%{IncludeDir.ECS}",
		"%{IncludeDir.VulkanMemoryAllocator}",
		"%{IncludeDir.Optick}",
		"%{IncludeDir.stb_image}"
	}

	links 
	{ 
		"GLFW",
		"ImGui",
		"Optick",
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

	filter "configurations:Shipping"
		defines "SHIPPING"
        optimize "on"
		symbols "off"