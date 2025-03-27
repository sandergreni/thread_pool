
#include "thread_pool.h"
#include <gtest/gtest.h>
#include <algorithm>
#include <chrono>
#include <print>
#include <random>
#include <ranges>

using namespace std::chrono_literals;

TEST(ThreadPoolTest, simple)
{
	// Create a thread pool with 4 threads
	xo::thread_pool::thread_pool_t pool(4);

	// Example of enqueueing tasks
	auto result1 = pool.enqueue([](std::stop_token stoken, int x) -> std::expected<int, xo::thread_pool::error_t> {
		if (stoken.stop_requested())
		{
			return std::unexpected(xo::thread_pool::error_t::stop_requested);
		}

		return x * x;
	}, 10);

	auto result2 = pool.enqueue([](std::stop_token stoken, std::string_view suffix) -> std::expected<std::string, xo::thread_pool::error_t> {
		if (stoken.stop_requested())
		{
			return std::unexpected(xo::thread_pool::error_t::stop_requested);
		}

		return std::string("Hello").append(suffix);
	}, std::string(" world"));

	auto result3 = pool.enqueue([](std::stop_token /*stoken*/) -> std::expected<std::string, xo::thread_pool::error_t> {
		return std::unexpected(xo::thread_pool::error_t::error);
	});

	// Get results
	auto const v1 = result1.get();
	ASSERT_TRUE(v1.has_value());
	ASSERT_EQ(v1.value(), 100);

	auto const v2 = result2.get();
	ASSERT_TRUE(v2.has_value());
	ASSERT_EQ(v2.value(), "Hello world");

	auto const v3 = result3.get();
	ASSERT_TRUE(!v3.has_value());
	ASSERT_EQ(v3.error(), xo::thread_pool::error_t::error);
}

int main(int argc, char* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
} 
