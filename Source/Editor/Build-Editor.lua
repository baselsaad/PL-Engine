-- Engine-Editor
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
		"PROJECT_ROOT=\"" .. ProjectRoot .. "\""
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
		"%{wks.location}/Source/Engine/src",
		"%{wks.location}/Source/Engine/vendor",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.imgui}",
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
		"Engine"
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
