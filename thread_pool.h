#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <iostream>
#include <string>
#include <queue>
#include <functional>
#include <memory>
#include <thread>
#include <future>

class TaskQueue
{
public:
	using Task = std::function<void()>;

	void Enqueue(Task& task)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		queue_.emplace(task);
	}

	bool Dequeue(Task& task)
	{
		std::unique_lock<std::mutex> lock(mtx_);

		if (queue_.empty())
			return false;

		task = queue_.front();
		queue_.pop();

		return true;
	}

	bool Empty()
	{
		std::unique_lock<std::mutex> lock(mtx_);

		return queue_.empty();
	}

	size_t Size()
	{
		std::unique_lock<std::mutex> lock(mtx_);

		return queue_.size();
	}

private:
	std::queue<Task> queue_;
	std::mutex mtx_;
};

class ThreadPool
{
	class Worker
	{
	public:
		Worker(ThreadPool& pool, int id) : pool_(pool), id_(id)
		{

		}

		void Run()
		{
			TaskQueue::Task task;
			bool dequeue = false;

			while (!pool_.shutdown_)
			{
				// execute task time after release lock
				{
					std::unique_lock<std::mutex> lock(pool_.mtx_);
					if (pool_.task_queue_.Empty())
					{
						pool_.cv_.wait(lock);
					}

					dequeue = pool_.task_queue_.Dequeue(task);
				}

				// execute task
				if (dequeue)
					task();
			}
		}

	private:
		ThreadPool& pool_;
		int id_;
	};

public:
	ThreadPool(int thread_num = 1) : shutdown_(false), thread_group_(std::vector<std::thread>(thread_num))
	{

	}

	void Start()
	{
		for (size_t i = 0; i < thread_group_.size(); ++i)
		{
			thread_group_[i] = std::thread(std::bind(&Worker::Run, new Worker(*this, i)));
		}
	}

	template<typename F, typename... Args>
	auto Submit(F&& func, Args&&... args) -> std::future<decltype(func(args...))>
	{
		using ReturnType = decltype(func(args...));

		std::function<ReturnType()> f = std::bind(std::forward<F>(func), std::forward<Args>(args)...);

		auto task = std::make_shared<std::packaged_task<ReturnType()>>(f);

		TaskQueue::Task wrapper_task = [=]() {
			(*task)();
		};

		task_queue_.Enqueue(wrapper_task);
		cv_.notify_one();

		return task->get_future();
	}

	void Shutdown()
	{
		shutdown_ = true;
		cv_.notify_all();

		for (auto& t : thread_group_)
		{
			t.join();
		}
	}

public:
	bool shutdown_;

	TaskQueue task_queue_;
	std::vector<std::thread> thread_group_;

	std::condition_variable cv_;
	std::mutex mtx_;
};

#endif // !_THREAD_POOL_H_