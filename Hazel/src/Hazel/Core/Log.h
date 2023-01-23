#pragma once

#include "Hazel/Core/Base.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#pragma warning(push, 0)
#include <filesystem>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/format.h>
#pragma warning(pop)

namespace std::filesystem
{
	class path;
}

namespace Hazel
{
	class Log
	{
	public:
		static void Init();

		static Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static Ref<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
	};
}

template<glm::length_t L, typename T, glm::qualifier Q>
struct fmt::formatter<glm::vec<L, T, Q>> {
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
		return ctx.end();
	}

	template <typename FormatContext>
	auto format(const glm::vec<L, T, Q>& vector, FormatContext& ctx) -> decltype(ctx.out()) {
		return format_to(ctx.out(), glm::to_string(vector));
	}
};

template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct fmt::formatter<glm::mat<C, R, T, Q>> {
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
		return ctx.end();
	}

	template <typename FormatContext>
	auto format(const glm::mat<C, R, T, Q>& matrix, FormatContext& ctx) -> decltype(ctx.out()) {
		return format_to(ctx.out(), glm::to_string(matrix));
	}
};

template<typename T, glm::qualifier Q>
struct fmt::formatter<glm::qua<T, Q>> {
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
		return ctx.end();
	}

	template <typename FormatContext>
	auto format(const glm::qua<T, Q>& quaternion, FormatContext& ctx) -> decltype(ctx.out()) {
		return format_to(ctx.out(), glm::to_string(quaternion));
	}
};

template<>
struct fmt::formatter<std::filesystem::path> : formatter<std::string_view> {
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
		return ctx.end();
	}

	template <typename FormatContext>
	auto format(const std::filesystem::path& path, FormatContext& ctx) -> decltype(ctx.out()) {
		return formatter<std::string_view>::format(path.string(), ctx);
	}
};

// Core log macros
#define HZ_CORE_TRACE(...)		::Hazel::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define HZ_CORE_INFO(...)		::Hazel::Log::GetCoreLogger()->info(__VA_ARGS__)
#define HZ_CORE_WARN(...)		::Hazel::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define HZ_CORE_ERROR(...)		::Hazel::Log::GetCoreLogger()->error(__VA_ARGS__)
#define HZ_CORE_CRITICAL(...)	::Hazel::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define HZ_TRACE(...)			::Hazel::Log::GetClientLogger()->trace(__VA_ARGS__)
#define HZ_INFO(...)			::Hazel::Log::GetClientLogger()->info(__VA_ARGS__)
#define HZ_WARN(...)			::Hazel::Log::GetClientLogger()->warn(__VA_ARGS__)
#define HZ_ERROR(...)			::Hazel::Log::GetClientLogger()->error(__VA_ARGS__)
#define HZ_CRITICAL(...)		::Hazel::Log::GetClientLogger()->critical(__VA_ARGS__)