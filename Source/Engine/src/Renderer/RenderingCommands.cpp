#include "pch.h"
#include "RenderingCommands.h"


namespace PAL
{
	CommandsQueue::CommandsQueue(size_t size /*= 2 * 1E+6*/)
		: m_BufferSize(size)
		, m_BufferUsed(0)
	{
		m_Buffer = new uint8_t[size];
		memset(m_Buffer, 0, size);
	}

	CommandsQueue::~CommandsQueue()
	{
		delete[] m_Buffer;
	}

	void CommandsQueue::ExecuteAll()
	{
		uint32_t offset = 0;

		while (offset < m_BufferUsed)
		{
			ICommand* cmd = reinterpret_cast<ICommand*>(m_Buffer + offset);
			cmd->Execute();
			offset += cmd->Size();
		}
	}

	void CommandsQueue::Clear()
	{
		uint32_t offset = 0;

		while (offset < m_BufferUsed)
		{
			ICommand* cmd = reinterpret_cast<ICommand*>(m_Buffer + offset);
			cmd->~ICommand();
			offset += cmd->Size();
		}

		m_BufferUsed = 0;
		m_TotalCommandsCount = 0;
	}

}
