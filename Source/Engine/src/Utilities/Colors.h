#pragma once
#include <vector>

namespace PAL
{
	struct RGBA
	{
		float R, G, B, Alpha;

		RGBA(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f)
			: R(r)
			, G(g)
			, B(b)
			, Alpha(a)
		{
		}

		RGBA(const RGBA& rgba)
			: R(rgba.R)
			, G(rgba.G)
			, B(rgba.B)
			, Alpha(rgba.Alpha)
		{
		}

		RGBA(const glm::vec4& rgba)
			: R(rgba.x)
			, G(rgba.y)
			, B(rgba.z)
			, Alpha(rgba.w)
		{
		}

		bool operator==(const RGBA& other)
		{
			return
				R == other.R
				&& G == other.G
				&& B == other.B
				&& Alpha == other.Alpha;
		}

		operator glm::vec3()
		{
			return { R,G,B };
		}

		operator glm::vec4()
		{
			return { R,G,B, Alpha};
		}

		operator const glm::vec3() const 
		{
			return { R,G,B };
		}

		operator const glm::vec4() const
		{
			return { R,G,B,Alpha };
		}

		glm::vec3 GetVec3() const 
		{
			return { R,G,B };
		}
		
	};

	struct Colors
	{
		inline static const RGBA Red{ 0.77f, 0.12f, 0.23f };				//#C41E3A
		inline static const RGBA Dark_Magenta{ 0.64f, 0.19f, 0.79f };		//#A330C9
		inline static const RGBA Orange{ 0.89f, 0.49f, 0.04f };				//#FF7C0A
		inline static const RGBA Dark_Cyan{ 0.20f, 0.58f, 0.50f };			//#33937F
		inline static const RGBA Green{ 0.67f, 0.83f, 0.45f };				//#AAD372
		inline static const RGBA Light_Blue{ 0.25f, 0.78f, 0.92f };			//#3FC7EB
		inline static const RGBA Spring_Green{ 0.00f, 1.00f, 0.60f };		//#00FF98
		inline static const RGBA Pink{ 0.96f, 0.55f, 0.73f };				//#F48CBA
		inline static const RGBA Yellow{ 1.00f, 0.96f, 0.41f };				//#FFF468
		inline static const RGBA Blue{ 0.00f, 0.44f, 0.87f };				//#0070DD
		inline static const RGBA Purple{ 0.53f, 0.53f, 0.93f };				//#8788EE
		inline static const RGBA Tan{ 0.78f, 0.61f, 0.43f };				//#C69B6D
		inline static const RGBA White{ 1.00f, 1.00f, 1.00f };				//#FFFFFF
		inline static const RGBA Black{ 0.00f, 0.00f, 0.00f };				//#000000

		inline static const std::vector<RGBA> Array =
		{
		  Red
		, Dark_Magenta
		, Orange
		, Dark_Cyan
		, Green
		, Light_Blue
		, Spring_Green
		, Pink
		, White
		, Yellow
		, Blue
		, Purple
		, Tan
		};

	};

	static const RGBA HighLightColor{ 0.77f, 0.12f, 0.23f, 0.4f };
}




