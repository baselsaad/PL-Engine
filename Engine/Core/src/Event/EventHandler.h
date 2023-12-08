#pragma once
#include "Event.h"

namespace PL_Engine
{
	using EventFuncType = std::function<void(const Event&)>;
	// @TODO: BindActions and Axsis

	class EventHandler
	{
	public:
		EventHandler() = default;
		~EventHandler() = default;

	public:
		// Method parameter should be Drived_From_Event 
		template<typename ObjectType, typename DrivedFromEvent>
		void BindAction(EventType inputEvent, ObjectType* obj, void (ObjectType::* func)(const DrivedFromEvent&))
		{
			static_assert(std::is_base_of<Event, DrivedFromEvent>::value, "Parameter Type of the Method must be derived from Event!!");

			EventFuncType lambda = [obj, func](const Event& e) -> void
			{
				(obj->*func)(static_cast<const DrivedFromEvent&>(e));
			};

			m_BoundFunctions[inputEvent].push_back(lambda);
		}

	private:
		friend class Engine;
		void OnEvent(Event& e);

		std::unordered_map<EventType, std::vector<EventFuncType>> m_BoundFunctions;
	};
}