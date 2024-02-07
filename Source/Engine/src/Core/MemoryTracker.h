#pragma once

namespace PAL
{
	struct MapEntry
	{
		void* Key;
		size_t Value;
	};

	class SimpleMap
	{
	public:
		static constexpr size_t NOT_FOUND = -1;

		SimpleMap()
			: m_Size(0)
			, m_Capacity(100)
		{
			m_Entries = static_cast<MapEntry*>(malloc(sizeof(MapEntry) * m_Capacity));
			if (!m_Entries)
			{
				throw std::bad_alloc();
			}
		}

		~SimpleMap()
		{
			free(m_Entries);
		}

		void Insert(void* ptr, size_t size)
		{
			if (m_Size >= m_Capacity)
			{
				Resize(m_Capacity * 2);
			}

			m_Entries[m_Size].Key = ptr;
			m_Entries[m_Size].Value = size;
			m_Size++;
		}

		MapEntry Find(void* ptr) const
		{
			for (size_t i = 0; i < m_Size; ++i)
			{
				if (m_Entries[i].Key == ptr)
				{
					return m_Entries[i];
				}
			}

			return { nullptr, NOT_FOUND };
		}

		void Remove(void* ptr)
		{
			for (size_t i = 0; i < m_Size; ++i)
			{
				if (m_Entries[i].Key == ptr)
				{
					// Shift all subsequent elements one position to the left
					for (size_t j = i; j < m_Size - 1; ++j)
					{
						m_Entries[j] = m_Entries[j + 1];
					}

					m_Size--;
					return;
				}
			}
		}

		inline size_t GetCapacity() { return m_Capacity; }

	private:
		MapEntry* m_Entries;
		size_t m_Size;
		size_t m_Capacity;

		void Resize(size_t newCapacity)
		{
			MapEntry* newEntries = static_cast<MapEntry*>(realloc(m_Entries, sizeof(MapEntry) * newCapacity));
			if (newEntries)
			{
				m_Entries = newEntries;
				m_Capacity = newCapacity;
			}
			else
			{
				// Handle realloc failure, potentially throwing an exception or handling it as per requirements
			}
		}
	};

	class AllocationTracker
	{
	public:
		AllocationTracker() = delete;
		AllocationTracker(const AllocationTracker&) = delete;
		AllocationTracker(AllocationTracker&&) = delete;

		static void Init();
		static void Shutdown();

		static void* Allocate(size_t const size);
		static void Free(void* const block);

		inline static float GetCurrentUsage() { return Usage / 1000000.0f; }
		inline static float GetTotalAllocated() { return TotalAllocated / 1000000.0f; }
		inline static float GetTotalFreed() { return TotalFreed / 1000000.0f; }

	private:
		inline static size_t TotalAllocated = 0;
		inline static size_t Usage = 0;
		inline static size_t TotalFreed = 0;
		inline static size_t TotalAllocationsCount = 0;
	};
}

#ifdef _MSC_VER // Check if using Microsoft Visual C++
#define MY_NODISCARD _NODISCARD
#define MY_RET_NOTNULL _Ret_notnull_
#define MY_POST_WRITABLE_BYTE_SIZE(size) _Post_writable_byte_size_(size)
#define MY_VCRT_ALLOCATOR _VCRT_ALLOCATOR
#define MY_CRTDECL __CRTDECL
#else
#define MY_NODISCARD
#define MY_RET_NOTNULL
#define MY_POST_WRITABLE_BYTE_SIZE(size)
#define MY_VCRT_ALLOCATOR
#endif

// no need to track placement new operator

DISABLE_WARNING_PUSH
DISABLE_WARNING(28252 28253)

MY_NODISCARD MY_RET_NOTNULL MY_POST_WRITABLE_BYTE_SIZE(size) MY_VCRT_ALLOCATOR
void* MY_CRTDECL operator new(size_t size);

// No-throw variants of new
MY_NODISCARD MY_RET_NOTNULL MY_POST_WRITABLE_BYTE_SIZE(size) MY_VCRT_ALLOCATOR
void* MY_CRTDECL operator new(size_t size, const std::nothrow_t&) noexcept;

MY_NODISCARD MY_RET_NOTNULL MY_POST_WRITABLE_BYTE_SIZE(size) MY_VCRT_ALLOCATOR
void* MY_CRTDECL operator new[](size_t size);

MY_NODISCARD MY_RET_NOTNULL MY_POST_WRITABLE_BYTE_SIZE(size) MY_VCRT_ALLOCATOR
void* MY_CRTDECL operator new[](size_t size, const std::nothrow_t&) noexcept;


void MY_CRTDECL operator delete(void* pointer) noexcept;
void MY_CRTDECL operator delete(void* pointer, const std::nothrow_t&) noexcept;
void MY_CRTDECL operator delete[](void* pointer) noexcept;
void MY_CRTDECL operator delete[](void* pointer, const std::nothrow_t&) noexcept;


DISABLE_WARNING_POP