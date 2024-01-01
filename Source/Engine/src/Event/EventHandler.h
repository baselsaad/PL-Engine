#pragma once
#include "Event.h"

namespace PAL
{
	using EventCallback = std::function<void(const Event&)>;
	// @TODO: BindActions and Axis

	class EventHandler
	{
	public:
		EventHandler() = default;
		~EventHandler() = default;

	public:
		// Binds a member function to a specific event type
		// 
		// @tparam EventType - Type of the event to bind
		// @tparam ObjectType - The class containing the member function
		// @tparam DrivedFromEvent - Parameter the member function should accept
		// 
		// @param eventType - Type of the event to bind
		// @param obj - Object instance of ObjectType to bind the member function to
		// @param memberFunction - Member function in ObjectType
		template<typename ObjectType, typename EventType, typename DrivedFromEvent>
		void BindAction(EventType eventType, ObjectType* obj, void (ObjectType::* memberFunction)(const DrivedFromEvent&))
		{
			// Ensure that DrivedFromEvent is derived from the Event base class
			static_assert(std::is_base_of<Event, DrivedFromEvent>::value, "Parameter type of the member function must be derived from Event!");

			EventCallback callback = [obj, memberFunction](const Event& e) -> void
			{
				(obj->*memberFunction)(static_cast<const DrivedFromEvent&>(e));
			};
			
			m_BoundCallbacks[eventType].push_back(callback);
		}


	private:
		friend class Engine;
		void OnEvent(Event& e);

		std::unordered_map<EventType, std::vector<EventCallback>> m_BoundCallbacks; // one event can have multiple callbacks 
	};
}