#pragma once

namespace PAL
{
	class ICommand
	{
	public:
		virtual ~ICommand() = default;
		virtual void Execute() = 0;
		virtual size_t Size() const = 0;
	};

	template<typename T>
	class Command : public ICommand
	{
	public:
		Command(T&& func) 
			: m_Func(std::move(func))
		{
		}

		void Execute() override
		{
			m_Func();
		}

		size_t Size() const override
		{
			return sizeof(Command<T>);
		}

	private:
		T m_Func;
	};

	class CommandsQueue
	{
	public:
		// size in bytes (for now 2 mb for commands, later make this dynamic) 
		CommandsQueue(size_t size = 2 * 1E+6);
		~CommandsQueue();

		template<typename T>
		void Record(T&& func)
		{
			uint32_t spaceNeeded = sizeof(Command<T>);
			ASSERT(m_BufferUsed + spaceNeeded <= m_BufferSize, "Not enough space to record the command!!!");
			
			void* place = m_Buffer + m_BufferUsed;
			ICommand* command = new (place) Command<T>(std::forward<T>(func));
			m_BufferUsed += spaceNeeded;
			m_TotalCommandsCount++;
		}

		void ExecuteAll();
		void Clear();

		inline uint32_t GetCommandsCount() const { return m_TotalCommandsCount; }
		inline uint32_t GetCommandsUsage() const { return m_BufferUsed; }

	private:
		uint8_t* m_Buffer;
		uint32_t m_BufferSize;
		uint32_t m_BufferUsed;
		uint32_t m_TotalCommandsCount;
	};
}
