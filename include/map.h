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

#ifndef MAP_H__
#define MAP_H__

#include <threadpool.h>
#include <algorithm>

using namespace threadpool;

template<typename M>
void map(std::vector<M> &v, std::function<void(M *)> f, ThreadCache &cache) {
	Threadpool<M *> pool(doNothing, f, doNothing, 10, 10, cache);
	std::for_each(v.begin(), v.end(), [&pool](M &e) { pool.add(&e); });
}

template<typename M, typename O>
std::vector<O> map(const std::vector<M> &v, std::function<M(const M *)> f, ThreadCache &cache) {
	std::vector<O> output(v.size());
	Threadpool<unsigned int> pool(doNothing, [&output, &v, f](unsigned int i) { output[i] = std::move(f(&v[i])); }, doNothing, 10, 10, cache);
	for (unsigned int i = 0; i < v.size(); i++) { pool.add(i); };
	return output;
}

#endif
