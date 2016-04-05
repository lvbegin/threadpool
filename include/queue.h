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

	void push(M &newValue) {
		std::unique_lock<std::mutex> lock(mutex);

		queueNotFull.wait(lock, [this, &newValue]() { return isTerminated || BoundedQueue<M>::push(newValue); });
		if (isTerminated)
			throw std::runtime_error("Cannot push in terminated queue.");
		queueNotEmpty.notify_one();
	}
	M *pop(void) {
		std::unique_lock<std::mutex> lock(mutex);

		M *value;
		queueNotEmpty.wait(lock, [this, &value]() { value = BoundedQueue<M>::pop(); return (nullptr != value || isTerminated); });
		if (!isTerminated)
			queueNotFull.notify_one();
		return value;
	}
	void terminate() {
		std::lock_guard<std::mutex> lock(mutex);

		isTerminated = true;
		queueNotEmpty.notify_all();
		queueNotFull.notify_all();
	}
	static bool isTerminatedMessage(const M *message) {
		return (nullptr == message);
	}

private:
	std::mutex mutex;
	std::condition_variable queueNotEmpty;
	std::condition_variable queueNotFull;
	bool isTerminated;
};

}
#endif
