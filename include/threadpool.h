/* Copyright 2015 Laurent Van Begin
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
  */

#ifndef __THREADPOOL_H_
#define __THREADPOOL_H_

#include <functional>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

namespace threadpool {

template <typename T, typename M>
class Threadpool {
public:

	typedef struct {
		std::function<void(T *)> WorkerThreadInit;
		std::function<void(T *, M &)> WorkerThreadBody;
		std::function<void(T *)> WorkerThreadFinal;
	} WorkerThreadFunctions;
	explicit Threadpool(WorkerThreadFunctions &functions, T *threadContext, size_t poolSize, size_t waitingQueueSize);
	~Threadpool();

	void add(M &message);

private:
	typedef std::unique_ptr<std::thread> threadPtr;
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

	class BlockingBoundedQueue : private BoundedQueue {
	public:
		BlockingBoundedQueue(size_t maxValues) : BoundedQueue(maxValues), isTerminated(false) { }
		~BlockingBoundedQueue() = default;

		bool push(M &newValue) {
			std::unique_lock<std::mutex> lock(mutex);
			if (isTerminated)
				throw std::runtime_error("Cannot push in terminated queue.");
			const auto rc = BoundedQueue::push(newValue);
			if (rc)
				condition.notify_one();
			return rc;
		}
		M *pop(void) {
			std::unique_lock<std::mutex> lock(mutex);
			for ( ; ; ) {
				auto value = BoundedQueue::pop();
				if (nullptr != value || isTerminated)
					return value;
				condition.wait(lock);
			}
		}
		void terminate() {
			std::unique_lock<std::mutex> lock(mutex);
			isTerminated = true;
			condition.notify_all();
		}
		bool isTerminatedMessage(const M *message) {
			return (nullptr == message);
		}

	private:
		std::mutex mutex;
		std::condition_variable condition;
		bool isTerminated;
	};

	static void threadBody(WorkerThreadFunctions *functions,  T  *context, BlockingBoundedQueue *queue) {
		if (nullptr != functions->WorkerThreadInit)
			functions->WorkerThreadInit(context);
		for ( ; ; ) {
			auto message = queue->pop();
			if (queue->isTerminatedMessage(message))
				break;
			functions->WorkerThreadBody(context, *message);
		}
		if (nullptr != functions->WorkerThreadFinal)
			functions->WorkerThreadFinal(context);
	}
	std::vector<threadPtr> threads;
	BlockingBoundedQueue pendingMessages;
};

template <typename T, typename M>
Threadpool<T, M>::Threadpool(WorkerThreadFunctions &functions, T *threadContext, size_t poolSize, size_t waitingQueueSize) :
								threads(), pendingMessages(waitingQueueSize)  {
	for(size_t i = 0; i < poolSize; i++) {
		threads.push_back(std::make_unique<std::thread>(threadBody, &functions, threadContext, &pendingMessages));
	}
}

template <typename T, typename M>
Threadpool<T, M>::~Threadpool() {
	pendingMessages.terminate();
	for (auto& elem : threads) {
		elem->join();
	}
	threads.resize(0);
}

template <typename T, typename M>
void Threadpool<T, M>::add(M &message) {
	pendingMessages.push(message);
}

}

#endif
