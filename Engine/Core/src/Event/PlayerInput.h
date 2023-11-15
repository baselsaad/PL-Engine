#pragma once
#include "Event.h"

using EventFunc = std::function<void(Event&)>;

class PlayerInput
{
public:
	PlayerInput() = default;
	~PlayerInput() = default;

public:
	void OnEvent(Event& e);

	template<typename T, typename MethodParameter>
	inline void BindAction(EventType inputEvent, T* obj, void (T::* func)(const MethodParameter&))
	{
		static_assert(std::is_base_of<Event, MethodParameter>::value, "Parameter Type of the Method must be derived from Event!!");

		EventFunc lambda = [obj, func](const Event& e) 
		{ 
			(obj->*func)(static_cast<const MethodParameter&>(e));
		};

		m_BoundFunctions[inputEvent] = lambda;
	}

private:
	std::unordered_map<EventType, EventFunc> m_BoundFunctions;
};