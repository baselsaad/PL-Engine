#pragma once
#include <vector>

namespace Colors
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


		bool operator==(const RGBA& other)
		{
			return
				R == other.R
				&& G == other.G
				&& B == other.B
				&& Alpha == other.Alpha;
		}
	};


	static const RGBA Red{ 0.77f, 0.12f, 0.23f };				//#C41E3A
	static const RGBA Dark_Magenta{ 0.64f, 0.19f, 0.79f };		//#A330C9
	static const RGBA Orange{ 0.89f, 0.49f, 0.04f };			//#FF7C0A
	static const RGBA Dark_Cyan{ 0.20f, 0.58f, 0.50f };			//#33937F
	static const RGBA Green{ 0.67f, 0.83f, 0.45f };				//#AAD372
	static const RGBA Light_Blue{ 0.25f, 0.78f, 0.92f };		//#3FC7EB
	static const RGBA Spring_Green{ 0.00f, 1.00f, 0.60f };		//#00FF98
	static const RGBA Pink{ 0.96f, 0.55f, 0.73f };				//#F48CBA
	static const RGBA Yellow{ 1.00f, 0.96f, 0.41f };			//#FFF468
	static const RGBA Blue{ 0.00f, 0.44f, 0.87f };				//#0070DD
	static const RGBA Purple{ 0.53f, 0.53f, 0.93f };			//#8788EE
	static const RGBA Tan{ 0.78f, 0.61f, 0.43f };				//#C69B6D
	static const RGBA White{ 1.00f, 1.00f, 1.00f };				//#FFFFFF
	static const RGBA Black{ 0.00f, 0.00f, 0.00f };				//#000000

	static const RGBA HighLightColor{ 0.77f, 0.12f, 0.23f, 0.4f };

	static const std::vector<RGBA> ColorsArray =
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
}



