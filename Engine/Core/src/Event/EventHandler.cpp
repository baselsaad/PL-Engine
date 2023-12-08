#include "pch.h"
#include "EventHandler.h"
#include "Utilities/Timer.h"

namespace PL_Engine
{

	void EventHandler::OnEvent(Event& e)
	{
		auto finder = m_BoundFunctions.find(e.GetEventType());
		if (finder != m_BoundFunctions.end())
		{
			const std::vector<EventFuncType>& functions = finder->second;

			for (const EventFuncType& func : functions)
			{
				func(e);
			}
		}
	}

}