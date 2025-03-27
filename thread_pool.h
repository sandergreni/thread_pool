#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <condition_variable>
#include <future>
#include <ranges>
#include <algorithm>
#include <expected>

namespace xo::thread_pool
{
	enum class error_t
	{
		error,
		stop_requested,
	};

	class thread_pool_t
	{
		using task_t = std::function<void(std::stop_token)>;
		using tasks_t = std::queue<task_t>;

	public:
		thread_pool_t(size_t thread_count);
		thread_pool_t(thread_pool_t const&) = delete;
		thread_pool_t(thread_pool_t&&) = delete;
		thread_pool_t& operator=(thread_pool_t const&) = delete;
		thread_pool_t& operator=(thread_pool_t&&) = delete;
		~thread_pool_t();

		template<typename F, typename... Args>
		auto enqueue(F&& f, Args... args)
		{
			using return_t = typename std::invoke_result_t<F, std::stop_token, Args...>;

			auto new_task = std::make_shared<std::packaged_task<return_t(std::stop_token)>>(std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...));

			auto res = new_task->get_future();
			{
				std::unique_lock<std::mutex> lock(queue_mutex);

				tasks.emplace([new_task](std::stop_token stoken) mutable {
					(*new_task)(stoken);
				});
			}

			condition.notify_one();
			return res;
		}

	private:
		std::mutex queue_mutex;
		std::condition_variable_any condition;
		std::vector<std::jthread> workers;
		tasks_t tasks;
	};
}

#endif // THREAD_POOL_H
