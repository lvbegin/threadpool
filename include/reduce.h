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

#ifndef REDUCE_H__
#define REDUCE_H__

#include <queue>

using namespace threadpool;

template<typename M>
M reduce(const std::vector<M> &v, M initialValue, std::function<M (std::pair<const M, const M>)> f, ThreadCache &cache) {
	struct ReduceData {
		std::pair<const M, const M> value;
		ThreadSafeBoundedQueue<M> &q;
		ReduceData(M&& v1, M&& v2, ThreadSafeBoundedQueue<M> &queue) : value(v1, v2), q(queue) {}
		ReduceData(const M& v1, const M& v2, ThreadSafeBoundedQueue<M> &queue) : value(v1, v2), q(queue) {}
	};
	if (0 == v.size())
		return initialValue;
	unsigned int pending {1};
	ThreadSafeBoundedQueue<M> q;
	Threadpool<ReduceData> pool(doNothing, [f] (ReduceData data) { data.q.push(f(std::move(data.value))); }, doNothing, 10, 10, cache);
	pool.add(ReduceData(v[0], std::move(initialValue), q));
	for (; pending < v.size() / 2; pending++) { pool.add(ReduceData(v[pending], v[pending + 1], q)); }
	for (; 1 < pending; pending--) { pool.add(ReduceData(std::move(q.pop()), std::move(q.pop()), q)); }
	return (0 == (v.size() + 1) % 2) ? q.pop() : f(std::pair<const M, const M>(q.pop(), v[v.size() - 1]));
}


#endif
