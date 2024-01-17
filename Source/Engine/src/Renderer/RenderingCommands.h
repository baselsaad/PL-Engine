#pragma once

namespace PAL
{
	class Command
	{
	public:
		virtual ~Command() = default;
		virtual void Execute() = 0;
		virtual size_t Size() const = 0;
	};

	template<typename T>
	class DrawCommand : public Command
	{
	public:
		DrawCommand(T&& func)
			: m_Func(std::move(func))
		{
		}

		void Execute() override
		{
			m_Func();
		}

		size_t Size() const override
		{
			return sizeof(DrawCommand<T>);
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
			uint32_t spaceNeeded = sizeof(DrawCommand<T>);
			PAL_ASSERT(m_BufferUsed + spaceNeeded <= m_BufferSize, "try to allocate {} bytes, but the buffer has {} bytes", spaceNeeded, m_BufferSize - m_BufferUsed);

			void* place = m_Buffer + m_BufferUsed;
			Command* command = new (place) DrawCommand<T>(std::forward<T>(func));
			m_BufferUsed += spaceNeeded;
			m_TotalCommandsCount++;
		}

		void ExecuteAndClear();
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
