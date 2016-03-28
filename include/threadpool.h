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

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>

#include <queue.h>
#include <threadBody.h>
#include <threadCache.h>

namespace threadpool {

template <typename M>
class Threadpool {
public:
	explicit Threadpool(initFunction init, bodyFunction<M> body, finalFunction final, unsigned int poolSize, size_t waitingQueueSize) :
						pendingMessages(waitingQueueSize), threads() {
		for(size_t i = 0; i < poolSize; i++) {
			threads.push_back(std::make_unique<std::thread>(ThreadBody::run<M>,init, body, final, &pendingMessages));
		}
	}
	~Threadpool() {
		pendingMessages.terminate();
		std::for_each(threads.begin(), threads.end(), [](std::unique_ptr<std::thread> &t) { t->join(); });
	}
	void add(M &message) { pendingMessages.push(message); }
private:
	ThreadSafeBoundedQueue<M> pendingMessages;
	std::vector<std::unique_ptr<std::thread>> threads;
};

template <typename M>
class TemporaryThreadpool {
public:
	explicit TemporaryThreadpool(initFunction init, bodyFunction<M> body, finalFunction final, unsigned int poolSize, size_t waitingQueueSize, ThreadCache &cache) :
								pendingMessages(new ThreadSafeBoundedQueue<M>(waitingQueueSize)) {
		cache.get(poolSize, init, body, final, pendingMessages);
	}
	~TemporaryThreadpool() { pendingMessages->terminate(); }
	void add(M &message) { pendingMessages->push(message); }
private:
	std::shared_ptr<ThreadSafeBoundedQueue<M>> pendingMessages;
};

static const std::function<void ()> doNothing = []() { };

}

#endif
