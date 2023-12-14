#include "pch.h"
#include "EventHandler.h"
#include "Utilities/Timer.h"

namespace PAL
{

	void EventHandler::OnEvent(Event& e)
	{
		auto finder = m_BoundCallbacks.find(e.GetEventType());
		if (finder != m_BoundCallbacks.end())
		{
			const std::vector<EventCallback>& functions = finder->second;

			for (const EventCallback& func : functions)
			{
				func(e);
			}
		}
	}

}