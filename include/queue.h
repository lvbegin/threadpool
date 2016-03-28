#ifndef QUEUE_H__
#define QUEUE_H__

#include <mutex>
#include <queue>
#include <condition_variable>
#include <stdexcept>

namespace threadpool {

template <typename M>
class BoundedQueue {
public:
	~BoundedQueue() = default;

	bool push(M &newValue) {
		if (maxSize == content.size())
			return false;
		content.push(&newValue);
		return true;
	}
	M *pop(void) {
		if (0 == content.size())
			return NULL;
		auto value = content.front();
		content.pop();
		return value;
	}
protected:
	BoundedQueue(size_t maxValues) : content(), maxSize(maxValues) { }
private:
	std::queue<M *> content;
	const size_t maxSize;
};

template <typename M>
class ThreadSafeBoundedQueue : private BoundedQueue<M> {
public:
	ThreadSafeBoundedQueue(size_t maxValues) : BoundedQueue<M>(maxValues), isTerminated(false) { }
	~ThreadSafeBoundedQueue() = default;

	bool push(M &newValue) {
		std::lock_guard<std::mutex> lock(mutex);
		if (isTerminated)
			throw std::runtime_error("Cannot push in terminated queue.");
		const auto rc = BoundedQueue<M>::push(newValue);
		if (rc)
			condition.notify_one();
		return rc;
	}
	M *pop(void) {
		std::unique_lock<std::mutex> lock(mutex);
		for ( ; ; ) {
			auto value = BoundedQueue<M>::pop();
			if (nullptr != value || isTerminated)
				return value;
			condition.wait(lock);
		}
	}
	void terminate() {
		std::lock_guard<std::mutex> lock(mutex);
		isTerminated = true;
		condition.notify_all();
	}
	static bool isTerminatedMessage(const M *message) {
		return (nullptr == message);
	}

private:
	std::mutex mutex;
	std::condition_variable condition;
	bool isTerminated;
};

}
#endif
