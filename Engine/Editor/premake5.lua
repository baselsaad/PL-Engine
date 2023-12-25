-- PL-Engine Editor
project "Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/Binaries/Engine/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/Intermediate/Engine/" .. outputdir .. "/%{prj.name}")

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"ENGINE_ROOT=\"" .. EngineRoot .. "\""
	}

	files
	{
		"src/*",
		"src/**.h",
		"src/**.hpp",
		"src/**.cpp",
	}

	includedirs
	{
		"src",
		"%{wks.location}/Engine/Core/src",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.Instrumentation}",
		"%{IncludeDir.Instrumentation}",
		"%{IncludeDir.Optick}",
		"%{IncludeDir.stb_image}"
	}

	links
	{
		"Core"
	}

	filter "system:windows"
		systemversion "latest"

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
