#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <condition_variable>
#include <future>
#include <ranges>
#include <algorithm>

namespace xo::thread_pool
{
	class thread_pool_t
	{
		using task_t = std::function<void(std::stop_token)>;
		using tasks_t = std::queue<task_t>;

	public:
		thread_pool_t(size_t thread_count)
		{
			std::ranges::for_each(std::views::iota(0, static_cast<int>(thread_count)), [this](auto)
				{
					workers.emplace_back([this](std::stop_token stoken)
						{
							while (!stoken.stop_requested())
							{
								task_t task;
								{
									std::unique_lock<std::mutex> lock(queue_mutex);
									bool const there_is_work_to_do = condition.wait(lock, stoken, [this]
										{
											return !tasks.empty();
										});

									if (stoken.stop_requested())
									{
										break;
									}
									else if (there_is_work_to_do)
									{
										task = std::move(tasks.front());
										tasks.pop();
									}
								}

								if (task)
								{
									if (stoken.stop_requested())
									{
										break;
									}

									task(stoken);
								}
							}
						});
				});
		}

		thread_pool_t(thread_pool_t const&) = delete;
		thread_pool_t(thread_pool_t&&) = delete;
		thread_pool_t& operator=(thread_pool_t const&) = delete;
		thread_pool_t& operator=(thread_pool_t&&) = delete;

		~thread_pool_t()
		{
			{
				std::unique_lock<std::mutex> lock(queue_mutex);
				tasks_t tmp;
				tmp.swap(tasks); // In the absense of clear()
			}

			std::ranges::for_each(workers, [this](std::jthread& worker)
				{
					worker.request_stop();
				});
		}

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
