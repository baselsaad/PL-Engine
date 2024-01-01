#pragma once
#include <GLFW/glfw3.h>

namespace PAL
{
#define BIND_FUN(obj, fun)											\
[&] (auto&&... args)												\
{																	\
	obj->fun(std::forward<decltype(args)>(args)...);				\
}

	enum class MouseButtonKey : uint16_t
	{
		Left = GLFW_MOUSE_BUTTON_LEFT, Right = GLFW_MOUSE_BUTTON_RIGHT, Middle = GLFW_MOUSE_BUTTON_MIDDLE, UnSupported
	};

	enum class KeyCode : uint16_t
	{
		// From glfw3.h
		Space = 32,
		Apostrophe = 39, /* ' */
		Comma = 44, /* , */
		Minus = 45, /* - */
		Period = 46, /* . */
		Slash = 47, /* / */

		D0 = 48, /* 0 */
		D1 = 49, /* 1 */
		D2 = 50, /* 2 */
		D3 = 51, /* 3 */
		D4 = 52, /* 4 */
		D5 = 53, /* 5 */
		D6 = 54, /* 6 */
		D7 = 55, /* 7 */
		D8 = 56, /* 8 */
		D9 = 57, /* 9 */

		Semicolon = 59, /* ; */
		Equal = 61, /* = */

		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		LeftBracket = 91,  /* [ */
		Backslash = 92,  /* \ */
		RightBracket = 93,  /* ] */
		GraveAccent = 96,  /* ` */

		World1 = 161, /* non-US #1 */
		World2 = 162, /* non-US #2 */

		/* Function keys */
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,

		/* Keypad */
		KP0 = 320,
		KP1 = 321,
		KP2 = 322,
		KP3 = 323,
		KP4 = 324,
		KP5 = 325,
		KP6 = 326,
		KP7 = 327,
		KP8 = 328,
		KP9 = 329,
		KPDecimal = 330,
		KPDivide = 331,
		KPMultiply = 332,
		KPSubtract = 333,
		KPAdd = 334,
		KPEnter = 335,
		KPEqual = 336,

		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348
	};


	enum class EventType
	{
		CloseWindow = 0,
		ResizeWindow,
		FrameBufferResize,
		MouseButton,
		MouseButtonPressed,
		MouseButtonReleased,
		MouseMiddleButtone,
		MouseScrolled,
		MouseMove,
		KeyRelease,
		KeyPressed,
		KeyRepeat
	};

	class Event
	{
	public:
		virtual EventType GetEventType() = 0;
	};

	class CloseWindowEvent : public Event
	{
		virtual EventType GetEventType() override
		{
			return EventType::CloseWindow;
		}
	};

	class ResizeWindowEvent : public Event
	{

	public:
		ResizeWindowEvent(int width, int height)
			: m_Width(width), m_Height(height)
		{
		}

		virtual EventType GetEventType() override
		{
			return EventType::ResizeWindow;
		}

		inline int GetWidth() const { return m_Width; }
		inline int GetHeight() const { return m_Height; }

	private:
		int m_Width, m_Height;

	};

	class ResizeFrameBufferEvent : public Event
	{

	public:
		ResizeFrameBufferEvent(int width, int height)
			: m_Width(width), m_Height(height)
		{
		}

		virtual EventType GetEventType() override
		{
			return EventType::FrameBufferResize;
		}

		inline int GetWidth() const { return m_Width; }
		inline int GetHeight() const { return m_Height; }

	private:
		int m_Width, m_Height;

	};

	class MouseButtonEvent : public Event
	{
	public:
		using Super = MouseButtonEvent;

		MouseButtonEvent(int button, double x, double y)
			: m_XPosition(x)
			, m_YPosition(y)
		{
			switch (button)
			{
				case GLFW_MOUSE_BUTTON_LEFT:	m_ButtonType = MouseButtonKey::Left;			break;
				case GLFW_MOUSE_BUTTON_RIGHT:	m_ButtonType = MouseButtonKey::Right;			break;
				case GLFW_MOUSE_BUTTON_MIDDLE:	m_ButtonType = MouseButtonKey::Middle;			break;
				default:						m_ButtonType = MouseButtonKey::UnSupported;		break;
			}
		}

		virtual EventType GetEventType() override
		{
			return EventType::MouseButton;
		}

		inline MouseButtonKey GetButtonType() { return m_ButtonType; }

		inline double GetXPosition() const { return m_XPosition; }

		// from BOTTOM_LEFT not TOP_LEFT
		inline double GetYPosition() const { return m_YPosition; }

	private:
		MouseButtonKey m_ButtonType;
		double m_XPosition;
		double m_YPosition;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(int btn, double x, double y)
			: Super(btn, x, y)
		{
		}

		virtual EventType GetEventType() override
		{
			return EventType::MouseButtonPressed;
		}
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{

	public:
		MouseButtonReleasedEvent(int btn, double x, double y)
			: Super(btn, x, y)
		{
		}

		virtual EventType GetEventType() override
		{
			return EventType::MouseButtonReleased;
		}
	};


	class MouseMoveEvent : public Event
	{

	public:
		MouseMoveEvent(double xPos, double yPos)
			: m_XPosition(xPos)
			, m_YPosition(yPos)
		{
		}

		inline double GetXPosition() const { return m_XPosition; }
		// from BOTTOM_LEFT not TOP_LEFT
		inline double GetYPosition() const { m_YPosition; }

		virtual EventType GetEventType() override
		{
			return EventType::MouseMove;
		}

	private:
		double m_XPosition;
		double m_YPosition;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(double xPos, double yPos)
			: m_XPosition(xPos)
			, m_YPosition(yPos)
		{
		}

		float GetXOffset() const { return m_XPosition; }
		float GetYOffset() const { return m_YPosition; }

		virtual EventType GetEventType() override
		{
			return EventType::MouseScrolled;
		}

	private:
		float m_XPosition, m_YPosition;
	};

	class KeyReleaseEvent : public Event 
	{
	public:
		KeyReleaseEvent(KeyCode key)
			: m_KeyCode(key)
		{
		}
		
		KeyCode GetPressedKey() const{ return m_KeyCode; }
		
		virtual EventType GetEventType() override
		{
			return EventType::KeyRelease;
		}

	private:
		KeyCode m_KeyCode;
	};

	class KeyPressedEvent : public Event
	{
	public:
		KeyPressedEvent(KeyCode key)
			: m_KeyCode(key)
		{
		}

		KeyCode GetPressedKey() const { return m_KeyCode; }

		virtual EventType GetEventType() override
		{
			return EventType::KeyPressed;
		}

	private:
		KeyCode m_KeyCode;
	};


	class KeyRepeatEvent : public Event
	{
	public:
		KeyRepeatEvent(KeyCode key)
			: m_KeyCode(key)
		{
		}

		KeyCode GetPressedKey() const { return m_KeyCode; }

		virtual EventType GetEventType() override
		{
			return EventType::KeyRepeat;
		}

	private:
		KeyCode m_KeyCode;
	};

}