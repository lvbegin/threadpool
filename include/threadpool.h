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

#ifndef THREADPOOL_H__
#define THREADPOOL_H__

#include <queue.h>
#include <threadCache.h>

namespace threadpool {

template <typename M>
class Threadpool {
public:
	explicit Threadpool(initFunction init, bodyFunction<M> body, finalFunction final, unsigned int poolSize, size_t waitingQueueSize) :
						cache(new ThreadCache(poolSize)), pendingMessages(waitingQueueSize), nbThreads(poolSize) {
		initializeThreads(init, body, final, poolSize, *cache);
	}
	explicit Threadpool(initFunction init, bodyFunction<M> body, finalFunction final, unsigned int poolSize, size_t waitingQueueSize, ThreadCache &threadCache) :
								cache(), pendingMessages(waitingQueueSize), nbThreads(poolSize) {
		initializeThreads(init, body, final, poolSize, threadCache);
	}
	~Threadpool() {
		std::unique_lock<std::mutex> lock(mutex);

		pendingMessages.terminate();
		allMessageTreated.wait(lock, [this]() { return (0 == nbThreads); });
	}
	void add(M message) { pendingMessages.push(std::move(message)); }
private:
	void initializeThreads(initFunction init, bodyFunction<M> body, finalFunction final, unsigned int poolSize, ThreadCache &cache) {
		auto termination = [this, f = std::move(final)]() { f(); notifyThreadFinalization(); };
		cache.get(poolSize, init, body, termination, pendingMessages);

	}
	void notifyThreadFinalization() {
		std::unique_lock<std::mutex> lock(mutex);

		nbThreads --;
		if (0 == nbThreads)
			allMessageTreated.notify_one();
	}
	std::mutex mutex;
	std::condition_variable allMessageTreated;
	std::unique_ptr<ThreadCache> cache;
	ThreadSafeBoundedQueue<M> pendingMessages;
	unsigned int nbThreads;
};

static const std::function<void ()> doNothing = []() { };

}

#endif
