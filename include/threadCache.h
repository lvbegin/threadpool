/* Copyright 2016 Laurent Van Begin
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

#ifndef THREAD_CACHE_H__
#define THREAD_CACHE_H__

#include <thread>
#include <mutex>

#include <threadBody.h>

namespace threadpool {

class ThreadCache {
public:
	ThreadCache(unsigned int nbThread) : size(nbThread) {
		const auto registration = [this](Thread *t) {
			std::lock_guard<std::mutex> lock(mutex);

			threads.push(std::unique_ptr<Thread>(t));
			if (threads.size() == size)
				condition.notify_one();
		};
		for (unsigned int i = 0; i < size; i++)
			threads.push(std::make_unique<Thread>(registration));
	}
	~ThreadCache() {
		std::unique_lock<std::mutex> lock(mutex);

		condition.wait(lock, [this]() { return size == threads.size(); });
	}
	template <typename M>
	void get(unsigned int nbThreads, initFunction init, bodyFunction<M> body, finalFunction final, std::shared_ptr<ThreadSafeBoundedQueue<M>> &queue) {
		std::unique_lock<std::mutex> lock(mutex);

		if (nbThreads > size)
			throw std::runtime_error("too much threads asked to cache");
		condition.wait(lock, [this, nbThreads]() { return nbThreads <= threads.size(); } );
		for (unsigned int i = 0; i < nbThreads; i++) {
			auto &thread = threads.front();
			thread->setParameters(init, body, final, queue);
			thread.release();
			threads.pop();
		}
	}
private:
	class Thread {
	public:
		Thread(std::function<void(Thread *)> registration) : state(threadState::NOT_INITIALIZED), mutex(), registration(registration), thread([this]() { cacheThreadBody(); }) {
			std::unique_lock<std::mutex> lock(mutex);

			condition.wait(lock, [this]() { return threadState::INITIALIZED == state; });
		}
		~Thread() {
			terminateThread();
			thread.join();
		};
		template <typename M>
		void setParameters(initFunction init, bodyFunction<M> body, finalFunction final, std::shared_ptr<ThreadSafeBoundedQueue<M>> queue) {
			std::lock_guard<std::mutex> lock(mutex);

			threadBody = [this, i = std::move(init), b = std::move(body), f = std::move(final), q = std::move(queue)]() { ThreadBody::run(*(&i), *(&b), *(&f), q.get()); };
			condition.notify_one();
		}
	private:
		enum class threadState { NOT_INITIALIZED, INITIALIZED, FINALIZED, };

		void terminateThread() {
			std::lock_guard<std::mutex> lock(mutex);

			state = threadState::FINALIZED;
			condition.notify_one();
		}
		void cacheThreadBody() {
			std::unique_lock<std::mutex> lock(mutex);

			state = threadState::INITIALIZED;
			condition.notify_one();
			for ( ; ; ) {
				condition.wait(lock);
				if (threadState::FINALIZED == state)
					return ;
				threadBody();
				registration(this);
			}
		}
		std::condition_variable condition;
		threadState state;
		std::mutex mutex;
		const std::function<void(Thread *)> registration;
		std::thread thread;
		std::function<void ()> threadBody;
	};

	const unsigned int size;
	std::mutex mutex;
	std::condition_variable condition;
	std::queue<std::unique_ptr<Thread>> threads;
};

}
#endif
