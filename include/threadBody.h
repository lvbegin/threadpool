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

#ifndef ABSTRACT_THREAD_H__
#define ABSTRACT_THREAD_H__

#include <queue.h>

namespace threadpool {

typedef std::function<void()> initFunction;

template <typename M>
using bodyFunction = std::function<void(M &)>;

typedef std::function<void()> finalFunction;

class ThreadBody {
public:
	ThreadBody() = delete;
	~ThreadBody() = delete;

	template <typename M>
	static void run(initFunction init, bodyFunction<M> body, finalFunction final, ThreadSafeBoundedQueue<M> *queue) { runByRef(init, body, final,  queue); }

	template <typename M>
	static void runByRef(const initFunction &init, const bodyFunction<M> &body, const finalFunction &final, ThreadSafeBoundedQueue<M> *queue) {
		init();
		for ( ; ; ) {
			auto message = queue->pop();
			if (queue->isTerminatedMessage(message))
				break;
			body(*message);
		}
		final();
	}

};

}

#endif
