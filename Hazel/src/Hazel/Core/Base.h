#pragma once

#include <memory>

#include "PlatformDetection.h"

#if defined(HZ_PLATFORM_WINDOWS)
	#define HZ_DEBUGBREAK() __debugbreak()
#elif defined(HZ_PLATFORM_LINUX)
	#include <signal.h>
	#define HZ_DEBUGBREAK() raise(SIGTRAP)
#else
	#error "Platform doesn't support debugbreak!'"
#endif

#if defined(HZ_DEBUG)
	#define HZ_ENABLE_ASSERTS
#endif

#ifndef HZ_DIST
	#define HZ_ENABLE_VERIFY
#endif

// DLL support
#if defined(HZ_PLATFORM_WINDOWS)
	#if HZ_DYNAMIC_LINK
		#ifdef HZ_BUILD_DLL
			#define HAZEL_API __declspec(dllexport)
		#else
			#define HAZEL_API __declspec(dllimport)
		#endif
	#else
		#define HAZEL_API
	#endif
#endif

#define HZ_EXPAND_MACRO(x) x
#define HZ_STRINGIFY_MACRO(x) #x

#define HZ_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define BIT(x) (1 << x)

namespace Hazel
{
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}

#include "Hazel/Core/Log.h"
#include "Hazel/Core/Assert.h"