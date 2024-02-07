#pragma once
#include <vector>
#include "imgui.h"
#include "Timer.h"

namespace PAL
{
#define R_Mask 0
#define G_Mask 8
#define B_Mask 16
#define A_Mask 24

	struct RGBA
	{
		uint32_t RGBAColor;
		
		RGBA(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255)
			: RGBAColor ((a << A_Mask) | (b << B_Mask) | (g << G_Mask) | (r << R_Mask))
		{
		}

		// Copy constructor
		RGBA(const RGBA& rgba)
			: RGBAColor(rgba.RGBAColor)
		{
		}

		RGBA(const glm::vec4& rgba)
			: RGBAColor( ((uint8_t)(rgba.a * 255.0f) << A_Mask) | ((uint8_t)(rgba.z * 255.0f) << B_Mask) | ((uint8_t)(rgba.y * 255.0f) << G_Mask) | (uint8_t)(rgba.x * 255.0f) << R_Mask)
		{
		}

		// Equality 
		bool operator==(const RGBA& other) const
		{
			return RGBAColor == other.RGBAColor;
		}

		// Conversion operator to glm::vec4
		operator glm::vec4() const
		{
			CORE_PROFILER_SCOPE("operator glm::vec4() const");

			return
			{
				static_cast<float>((RGBAColor >> R_Mask)  & 0xFF)  / 255.0f,
				static_cast<float>((RGBAColor >> G_Mask)  & 0xFF)  / 255.0f,
				static_cast<float>((RGBAColor >> B_Mask)  & 0xFF)  / 255.0f,
				static_cast<float>((RGBAColor >> A_Mask)  & 0xFF)  / 255.0f
			};
		}

		// Conversion operator to ImVec4
		operator ImVec4() const
		{
			CORE_PROFILER_SCOPE("operator ImVec4() const");

			return ImVec4
			(
				static_cast<float>((RGBAColor >> R_Mask) & 0xFF) / 255.0f,
				static_cast<float>((RGBAColor >> G_Mask) & 0xFF) / 255.0f,
				static_cast<float>((RGBAColor >> B_Mask) & 0xFF) / 255.0f,
				static_cast<float>((RGBAColor >> A_Mask) & 0xFF) / 255.0f
			);
		}
	};

	struct Colors
	{
		inline static const RGBA Red{ 196, 31, 58 };
		inline static const RGBA Dark_Magenta{ 163, 51, 201 };
		inline static const RGBA Orange{ 227, 125, 10 };
		inline static const RGBA Dark_Cyan{ 51, 148, 127 };
		inline static const RGBA Green{ 171, 211, 115 };
		inline static const RGBA Spring_Green{ 0, 255, 153 };
		inline static const RGBA Pink{ 245, 140, 186 };
		inline static const RGBA Yellow{ 255, 245, 105 };
		inline static const RGBA Blue{ 0, 112, 221 };
		inline static const RGBA Purple{ 135, 135, 237 };
		inline static const RGBA Tan{ 199, 156, 110 };
		inline static const RGBA White{ 255, 255, 255 };
		inline static const RGBA Black{ 0, 0, 0 };
		inline static const RGBA Gray{ 212, 212, 211 };

		// Light Variations
		inline static const RGBA Light_Red{ 236, 139, 159 };
		inline static const RGBA Light_Dark_Magenta{ 198, 74, 222 };
		inline static const RGBA Light_Orange{ 255, 194, 125 };
		inline static const RGBA Light_Dark_Cyan{ 125, 196, 181 };
		inline static const RGBA Light_Green{ 214, 240, 181 };
		inline static const RGBA Light_Light_Blue{ 148, 224, 255 };
		inline static const RGBA Light_Spring_Green{ 102, 255, 196 };
		inline static const RGBA Light_Pink{ 255, 199, 227 };
		inline static const RGBA Light_Yellow{ 255, 255, 194 };
		inline static const RGBA Light_Blue{ 164, 199, 245 };
		inline static const RGBA Light_Purple{ 196, 196, 248 };
		inline static const RGBA Light_Tan{ 228, 199, 164 };
		inline static const RGBA Light_White{ 255, 255, 255 };
		inline static const RGBA Light_Black{ 47, 47, 47 };

		// Dark Variations
		inline static const RGBA Dark_Red{ 156, 16, 43 };
		inline static const RGBA Dark_Dark_Magenta{ 128, 36, 151 };
		inline static const RGBA Dark_Orange{ 168, 91, 13 };
		inline static const RGBA Dark_Dark_Cyan{ 26, 105, 82 };
		inline static const RGBA Dark_Green{ 138, 176, 71 };
		inline static const RGBA Dark_Light_Blue{ 30, 153, 191 };
		inline static const RGBA Dark_Spring_Green{ 0, 204, 123 };
		inline static const RGBA Dark_Pink{ 210, 108, 151 };
		inline static const RGBA Dark_Yellow{ 215, 204, 71 };
		inline static const RGBA Dark_Blue{ 0, 74, 146 };
		inline static const RGBA Dark_Purple{ 91, 91, 168 };
		inline static const RGBA Dark_Tan{ 145, 102, 59 };
		inline static const RGBA Dark_Black{ 0, 0, 0 };
		inline static const RGBA Dark_Gray{ 121, 121, 121 };


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

}




