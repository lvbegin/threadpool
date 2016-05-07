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

#include <iostream>
#include <atomic>
#include <assert.h>

#include <threadpool.h>
#include <threadCache.h>

#include <map.h>
#include <reduce.h>

using namespace threadpool;

static void executeThreadPool__no_thread_context(int nbMessages)
{

	std::cout << "Execute threadpool without thread context: ";

	static const std::string message("Hello World");
	std::atomic<int> messageReceived {0};
	auto  t = std::make_unique<Threadpool<std::string>>(doNothing, [&messageReceived](const std::string m) {assert (m == message); messageReceived++;}, doNothing, 5, nbMessages);
	for (int i = 0; i < nbMessages; i++)
		t->add(message);

	t.reset(nullptr);
	if (messageReceived == nbMessages)
		std::cout << "OK" << std::endl;
	else
		std::cout << "NOK" << std::endl;
}

static void executeThreadPool__init_and_final(int nbMessages)
{
	std::cout << "Execute threadpool with init and final functions but without thread context: ";
	static const std::string message("Hello world");
	static std::atomic<unsigned int> initDone {0};
	static std::atomic<unsigned int> finalDone {0};

	auto init = []() { initDone++; };
	auto final = []() { finalDone++; };

	auto  t = std::make_unique<Threadpool<std::string>>(init, [](const std::string ) {}, final, 5, nbMessages);
	for (int i = 0; i < nbMessages; i++)
		t->add(message);

	t.reset(nullptr);
	if (5 == initDone.load() && 5 == finalDone.load())
		std::cout << "OK" << std::endl;
	else
		std::cout << "NOK" << std::endl;


}

static void executeThreadPool__use_external_thread_cache(int nbMessages)
{
	std::cout << "Execute threadpool with  external thread cache: ";

	static const std::string message("Hello World");
	std::atomic<int> messageReceived {0};
	auto cache = std::make_unique<ThreadCache>(2);

	auto  t = std::make_unique<Threadpool<std::string>>(doNothing, [&messageReceived](const std::string m) {assert (m == message); messageReceived++;}, doNothing, 1, nbMessages, *cache.get());
	for (int i = 0; i < nbMessages; i++)
		t->add(message);

	t.reset(nullptr);
	cache.reset(nullptr);
	if (messageReceived == nbMessages)
		std::cout << "OK" << std::endl;
	else
		std::cout << "NOK" << std::endl;
}

static void test_map_in_place(void)
{
	std::cout << "Test implementation of map in place operator: ";

	bool conclusion = true;
	std::vector<int> v(100, 0);
	ThreadCache cache(10);
	map<int>(v, [](int *i) { (*i)++; }, cache);

	for (size_t i = 0; i < v.size(); i++)
		if  (1 != v[i]) {
			std::cout << "i = " << i << ", v[i] = " << v[i] << std::endl;
			conclusion = false;
		}
	if (conclusion)
		std::cout << "OK" << std::endl;
	else
		std::cout << "NOK" << std::endl;
}

static void test_map(void)
{
	std::cout << "Test implementation of map operator: ";

	bool conclusion = true;
	std::vector<int> v(100, 0);
	ThreadCache cache(10);
	std::vector<int> output = map<int, int>(v, [](const int *i) { return (*i) + 1; }, cache);

	for (size_t i = 0; i < output.size(); i++)
		if  (1 != output[i]) {
			std::cout << "i = " << i << ", output[i] = " << output[i] << std::endl;
			conclusion = false;
		}
	if (conclusion)
		std::cout << "OK" << std::endl;
	else
		std::cout << "NOK" << std::endl;
}

static void test_map_with_pointers(void)
{
	std::cout << "Test implementation of map operator: ";

	bool conclusion = true;
	std::vector<std::unique_ptr<int>> v;
	ThreadCache cache(10);

	for(size_t i = 0; i < 100; i++) {  v.push_back(std::make_unique<int>(0)); };

	std::vector<std::unique_ptr<int>> output = map<std::unique_ptr<int>, std::unique_ptr<int>>(v, [](const std::unique_ptr<int> *i) {  return std::make_unique<int>(**i + 1); }, cache);


	for (size_t i = 0; i < output.size(); i++)
		if  (1 != *output[i]) {
			std::cout << "i = " << i << ", *output[i] = " << *output[i] << std::endl;
			conclusion = false;
		}
	if (conclusion)
		std::cout << "OK" << std::endl;
	else
		std::cout << "NOK" << std::endl;
}

static void test_associativeReduce(void)
{
	std::cout << "Test implementation of associativeReduce operator: ";

	std::vector<int> v(100, 1);
	ThreadCache cache(10);
	const auto response = associativeReduce<int>(v, 0, [](std::pair<const int, const int> v) { return v.first + v.second; }, cache);
	if (100 == response)
		std::cout << "OK" << std::endl;
	else
		std::cout << "NOK" << std::endl;
}

static void test_associativeReduce_with_one_element(void)
{
	std::cout << "Test implementation of associativeReduce operator: ";

	std::vector<int> v(1, 1);
	ThreadCache cache(10);
	const auto response = associativeReduce<int>(v, 0, [](std::pair<const int, const int> v) { return v.first + v.second; }, cache);
	if (1 == response)
		std::cout << "OK" << std::endl;
	else
		std::cout << "NOK" << std::endl;
}

int main()
{
	static const int nbMessages = 100000;
	executeThreadPool__no_thread_context(nbMessages);
	executeThreadPool__init_and_final(nbMessages);
	executeThreadPool__use_external_thread_cache(nbMessages);

	test_map_in_place();
	test_map();
	test_map_with_pointers();
	test_associativeReduce();
	test_associativeReduce_with_one_element();

	return EXIT_SUCCESS;
}
