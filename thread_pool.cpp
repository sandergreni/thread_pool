
#include "thread_pool.h"

namespace xo::thread_pool
{
	thread_pool_t::thread_pool_t(size_t thread_count)
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

	thread_pool_t::~thread_pool_t()
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
}