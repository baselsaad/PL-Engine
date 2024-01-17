#pragma once
#include <memory.h>

namespace PAL
{
	template<typename T>
	using WeakPtr = std::weak_ptr<T>;

	template<typename T>
	using UniquePtr = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr UniquePtr<T> NewUnique(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	//template<typename T>
	//using SharedPtr = std::shared_ptr<T>;


	template<class T>
	class SharedPtr
	{
	public:
		std::shared_ptr<T> StdSharedPtr;

		// Default constructor
		SharedPtr()
			: StdSharedPtr(nullptr)
		{
		}

		// Constructor that accepts a std::shared_ptr
		SharedPtr(std::shared_ptr<T> p)
			: StdSharedPtr(p)
		{
		}

		// Copy constructor
		SharedPtr(const SharedPtr<T>& other)
			: StdSharedPtr(other.StdSharedPtr)
		{
		}

		// Move constructor
		SharedPtr(SharedPtr<T>&& other) noexcept
			: StdSharedPtr(std::move(other.StdSharedPtr))
		{
		}

		template<typename T2>
		SharedPtr<T2> As()
		{
			return SharedPtr<T2>(std::static_pointer_cast<T2>(StdSharedPtr));
		}

		template<typename T2>
		SharedPtr<T2> As() const
		{
			return SharedPtr<T2>(std::static_pointer_cast<T2>(StdSharedPtr));
		}

		// Assignment operator for SharedPtr
		inline SharedPtr& operator=(const SharedPtr<T>& other)
		{
			if (this != &other)
			{
				StdSharedPtr = other.StdSharedPtr;
			}
			return *this;
		}

		// Assignment operator for std::shared_ptr
		inline SharedPtr& operator=(std::shared_ptr<T> other)
		{
			StdSharedPtr = std::move(other);
			return *this;
		}

		// Dereference operator
		inline T& operator*() const
		{
			return *StdSharedPtr;
		}

		// Arrow operator
		inline T* operator->() const
		{
			return StdSharedPtr.operator->();
		}

		// Equality operator
		inline bool operator==(const SharedPtr<T>& other) const
		{
			return StdSharedPtr == other.StdSharedPtr;
		}

		// Inequality operator
		inline bool operator!=(const SharedPtr<T>& other) const
		{
			return StdSharedPtr != other.StdSharedPtr;
		}

		// Boolean conversion
		inline explicit operator bool() const
		{
			return static_cast<bool>(StdSharedPtr);
		}

		// Comparison with nullptr
		inline bool operator==(std::nullptr_t) const
		{
			return StdSharedPtr == nullptr;
		}

		inline bool operator!=(std::nullptr_t) const
		{
			return StdSharedPtr != nullptr;
		}
	};


	template<typename T, typename ... Args>
	std::shared_ptr<T> NewShared(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<typename T2, typename T1>
	SharedPtr<T2> As(T1& obj)
	{
		return SharedPtr<T2>(std::static_pointer_cast<T2>(obj));
	}
}