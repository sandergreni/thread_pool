
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
	auto result1 = pool.enqueue([](std::stop_token stoken, int x) {
		if (stoken.stop_requested())
		{
			return 0;
		}

		return x * x;
	}, 10);

	auto result2 = pool.enqueue([](std::stop_token stoken) {
		if (stoken.stop_requested())
		{
			return std::string();
		}

		return std::string("Hello");
	});

	std::this_thread::sleep_for(1s);

	// Get results
	ASSERT_EQ(result1.get(), 100);
	ASSERT_EQ(result2.get(), "Hello");
}

int main(int argc, char* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
