#include <iostream>
#include <atomic>
#include <assert.h>

#include <threadpool.h>
#include <threadCache.h>

#include <map.h>

using namespace threadpool;

static void executeThreadPool__no_thread_context(int nbMessages)
{

	std::cout << "Execute threadpool without thread context: ";

	static const std::string message("Hello World");
	std::atomic<int> messageReceived {0};
	auto  t = std::make_unique<Threadpool<const std::string>>(doNothing, [&messageReceived](const std::string &m) {assert (m == message); messageReceived++;}, doNothing, 5, nbMessages);
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

	auto  t = std::make_unique<Threadpool<const std::string>>(init, [](const std::string &) {}, final, 5, nbMessages);
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

	auto  t = std::make_unique<Threadpool<const std::string>>(doNothing, [&messageReceived](const std::string &m) {assert (m == message); messageReceived++;}, doNothing, 1, nbMessages, *cache.get());
	for (int i = 0; i < nbMessages; i++)
		t->add(message);

	t.reset(nullptr);
	cache.reset(nullptr);
	if (messageReceived == nbMessages)
		std::cout << "OK" << std::endl;
	else
		std::cout << "NOK" << std::endl;
}

static void test_map(void)
{
	std::cout << "Test implementation of map operator: ";

	bool conclusion = true;
	std::vector<int> v(100, 0);
	ThreadCache cache(10); //how to ensure that threads terminated their job ?
	map<int>(v, [](int &i) { i++;}, cache);

//	for (size_t i = 0; i < v.size(); i++)
//		std::cout << v[i] << std::endl;
//
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

int main()
{
	static const int nbMessages = 100000;
	executeThreadPool__no_thread_context(nbMessages);
	executeThreadPool__init_and_final(nbMessages);
	executeThreadPool__use_external_thread_cache(nbMessages);

	test_map();
	return EXIT_SUCCESS;
}
