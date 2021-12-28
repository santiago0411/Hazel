#pragma once

#include "Hazel/Core/Base.h"
#include "Hazel/Core/Log.h"
#include <filesystem>

#define HZ_ENABLE_VERIFY

#ifdef HZ_ENABLE_ASSERTS

// Alternatively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
#define HZ_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { HZ##type##ERROR(msg, __VA_ARGS__); HZ_DEBUGBREAK(); } }
#define HZ_INTERNAL_ASSERT_WITH_MSG(type, check, ...) HZ_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define HZ_INTERNAL_ASSERT_NO_MSG(type, check) HZ_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", HZ_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define HZ_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define HZ_INTERNAL_ASSERT_GET_MACRO(...) HZ_EXPAND_MACRO( HZ_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, HZ_INTERNAL_ASSERT_WITH_MSG, HZ_INTERNAL_ASSERT_NO_MSG) )

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define HZ_ASSERT(...) HZ_EXPAND_MACRO( HZ_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
#define HZ_CORE_ASSERT(...) HZ_EXPAND_MACRO( HZ_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
#define HZ_ASSERT(...)
#define HZ_CORE_ASSERT(...)
#endif

#ifdef HZ_ENABLE_VERIFY
#define HZ_VERIFY_NO_MESSAGE(condition) { if(!(condition)) { HZ_ERROR("Verify Failed"); __debugbreak(); } }
#define HZ_VERIFY_MESSAGE(condition, ...) { if(!(condition)) { HZ_ERROR("Verify Failed: {0}", __VA_ARGS__); __debugbreak(); } }

#define HZ_VERIFY_RESOLVE(arg1, arg2, macro, ...) macro
#define HZ_GET_VERIFY_MACRO(...) HZ_EXPAND_VARGS(HZ_VERIFY_RESOLVE(__VA_ARGS__, HZ_VERIFY_MESSAGE, HZ_VERIFY_NO_MESSAGE))

#define HZ_VERIFY(...) HZ_EXPAND_VARGS( HZ_GET_VERIFY_MACRO(__VA_ARGS__)(__VA_ARGS__) )
#define HZ_CORE_VERIFY(...) HZ_EXPAND_VARGS( HZ_GET_VERIFY_MACRO(__VA_ARGS__)(__VA_ARGS__) )
#else
#define HZ_VERIFY(...)
#define HZ_CORE_VERIFY(...)
#endif