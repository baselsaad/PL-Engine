#include "pch.h"

#include <new>			// for std::bad_alloc
#include "MemoryTracker.h"

namespace PAL
{
	static std::mutex s_AllocationMutex;
	static SimpleMap* s_AllocationMap = nullptr;

	void AllocationTracker::Init()
	{
		if (s_AllocationMap == nullptr)
		{
			SimpleMap* data = (SimpleMap*)malloc(sizeof(SimpleMap));
			new(data) SimpleMap();
			s_AllocationMap = data;

			std::atexit(Shutdown); // Register Shutdown to be called at exit
		}
	}

	void AllocationTracker::Shutdown()
	{
		if (s_AllocationMap != nullptr)
		{
			s_AllocationMap->~SimpleMap(); // Explicitly call the destructor
			free(s_AllocationMap);         // Free the allocated memory
			s_AllocationMap = nullptr;
		}
	}

	void* AllocationTracker::Allocate(size_t const size)
	{
		std::lock_guard<std::mutex> guard(s_AllocationMutex);

		if (s_AllocationMap == nullptr)
		{
			Init();
		}

		void* block = malloc(size);
		if (block)
		{
			s_AllocationMap->Insert(block, size);
			TotalAllocated += size;
			Usage += size;
			TotalAllocationsCount++;
		}

		return block;
	}

	void AllocationTracker::Free(void* const block)
	{
		if (s_AllocationMap == nullptr)
			return;

		if (block == nullptr)
		{
			free(block);
			return;
		}

		std::lock_guard<std::mutex> guard(s_AllocationMutex);

		auto it = s_AllocationMap->Find(block);
		if (it.Value != s_AllocationMap->NOT_FOUND)
		{
			Usage -= it.Value;
			TotalFreed += it.Value;
			s_AllocationMap->Remove(it.Key);
		}
		else
		{
			PAL_ASSERT(false, "Memory block not in the map!!");
		}

		free(block);
	}
}

MY_NODISCARD MY_RET_NOTNULL MY_POST_WRITABLE_BYTE_SIZE(size) MY_VCRT_ALLOCATOR
void* MY_CRTDECL operator new(size_t size)
{
	void* ptr = PAL::AllocationTracker::Allocate(size);
	if (!ptr)
		throw std::bad_alloc();

	return ptr;
}

// No-throw variants of new
MY_NODISCARD MY_RET_NOTNULL MY_POST_WRITABLE_BYTE_SIZE(size) MY_VCRT_ALLOCATOR
void* MY_CRTDECL operator new(size_t size, const std::nothrow_t&) noexcept
{
	return PAL::AllocationTracker::Allocate(size);
}

MY_NODISCARD MY_RET_NOTNULL MY_POST_WRITABLE_BYTE_SIZE(size) MY_VCRT_ALLOCATOR
void* MY_CRTDECL operator new[](size_t size)
{
	return operator new(size); // Delegate to single-object new
}

MY_NODISCARD MY_RET_NOTNULL MY_POST_WRITABLE_BYTE_SIZE(size) MY_VCRT_ALLOCATOR
void* MY_CRTDECL operator new[](size_t size, const std::nothrow_t&) noexcept
{
	return operator new(size, std::nothrow);
}

void MY_CRTDECL operator delete(void* pointer) noexcept
{
	PAL::AllocationTracker::Free(pointer);
}

void MY_CRTDECL operator delete(void* pointer, const std::nothrow_t&) noexcept
{
	PAL::AllocationTracker::Free(pointer);
}

void MY_CRTDECL operator delete[](void* pointer, const std::nothrow_t&) noexcept
{
	operator delete(pointer, std::nothrow);
}

void MY_CRTDECL operator delete[](void* pointer) noexcept
{
	operator delete(pointer); // Delegate to single-object delete
}