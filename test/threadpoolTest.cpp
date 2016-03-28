#include <threadpool.h>
#include <iostream>
#include <atomic>

/*
 * 1. remove context
 * 2. improve tests by add test on counters.
 */

#include <threadCache.h>

using namespace threadpool;

class TestThreadBody__no_thread_context {

public:
	TestThreadBody__no_thread_context() = default;
	~TestThreadBody__no_thread_context() = default;

	void operator() (std::string &message)
	{
		if (message == reference_message)
			nbCalls++;
		return ;
	}
	static std::string reference_message;
	static std::atomic<int> nbCalls;
};

std::atomic<int> TestThreadBody__no_thread_context::nbCalls {0};
std::string TestThreadBody__no_thread_context::reference_message {"Hello world!"};

static std::atomic<unsigned int> initDone {0};
static std::atomic<unsigned int> finalDone {0};

auto init = []() { initDone++; };
auto final = []() { finalDone++; };

static void executeThreadPool__no_thread_context(int nbMessages)
{

	TestThreadBody__no_thread_context body;
	Threadpool<std::string> t(doNothing, body, doNothing, 5, nbMessages);
	for (int i = 0; i < nbMessages; i++)
		t.add(TestThreadBody__no_thread_context::reference_message);
}

static void executeThreadPool__init_and_final_but_no_thread_context(int nbMessages)
{

	TestThreadBody__no_thread_context body;
	Threadpool<std::string> t( init, body, final, 5, nbMessages);
	for (int i = 0; i < nbMessages; i++)
		t.add(TestThreadBody__no_thread_context::reference_message);

}

static void Cache__basic(int nbMessages)
{
	TestThreadBody__no_thread_context body;
	ThreadCache cache(2);
	TemporaryThreadpool<std::string> t(init, body, final, 1, nbMessages, cache);
	for (int i = 0; i < nbMessages; i++)
		t.add(TestThreadBody__no_thread_context::reference_message);
}

int main()
{
	static const int nbMessages = 100000;
	executeThreadPool__no_thread_context(nbMessages);
	std::cout << "Execute threadpool without thread context: ";
	if (TestThreadBody__no_thread_context::nbCalls.load() == nbMessages)
		std::cout << "OK" << std::endl;
	else
		std::cout << "NOK" << std::endl;

	executeThreadPool__init_and_final_but_no_thread_context(nbMessages);
	std::cout << "Execute threadpool with init and final functions but without thread context: ";
	if (5 == initDone.load() && 5 == finalDone.load())
		std::cout << "OK" << std::endl;
	else
		std::cout << "NOK" << std::endl;
	Cache__basic(nbMessages);
	if (TestThreadBody__no_thread_context::nbCalls.load() == (3 * nbMessages))
		std::cout << "OK" << std::endl;
	else
		std::cout << "NOK" << std::endl;

}
