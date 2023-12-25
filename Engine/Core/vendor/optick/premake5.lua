project "Optick"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
    staticruntime "on"

    targetdir ("%{wks.location}/Binaries/ThirdParty/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/Intermediate/ThirdParty/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp",
		"include/**.h"
	}

	includedirs
	{
		"%{VULKAN_SDK}/Include"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"
        symbols "off"