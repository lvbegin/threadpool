#include <threadpool.h>
#include <iostream>
#include <atomic>


using namespace threadpool;

class TestThreadBody__no_thread_context {

public:
	TestThreadBody__no_thread_context() = default;
	~TestThreadBody__no_thread_context() = default;

	void operator() (void *context, std::string &message)
	{
		if (message == reference_message && nullptr == context)
			nbCalls++;
		return ;
	}
	static std::string reference_message;
	static std::atomic<int> nbCalls;
};

std::atomic<int> TestThreadBody__no_thread_context::nbCalls {0};
std::string TestThreadBody__no_thread_context::reference_message {"Hello world!"};

class TestThreadBody__with_thread_context {

public:
	TestThreadBody__with_thread_context() {}
	~TestThreadBody__with_thread_context() = default;

	void operator() (std::string *context, std::string &message)
	{
		if (message == reference_message && reference_context == *context)
			nbCalls++;
		return ;
	}
	static std::string reference_message;
	static std::string reference_context;
	static std::atomic<unsigned int> nbCalls;
};

std::atomic<unsigned int> TestThreadBody__with_thread_context::nbCalls {0};
std::string TestThreadBody__with_thread_context::reference_message {"Hello world!"};
std::string TestThreadBody__with_thread_context::reference_context {"This is a dummy context"};

std::atomic<unsigned int> initDone {0};
std::atomic<unsigned int> finalDone {0};

void initFunction(void *)
{
	initDone++;
}

void finalFunction(void *)
{
	finalDone++;
}

static void executeThreadPool__no_thread_context(int nbMessages)
{
	TestThreadBody__no_thread_context body;
	Threadpool<void, std::string> t(Threadpool<void, std::string>::doNothing, body, Threadpool<void, std::string>::doNothing, nullptr, 5, nbMessages);
	for (int i = 0; i < nbMessages; i++)
		t.add(TestThreadBody__no_thread_context::reference_message);
}

static void executeThreadPool__init_and_final_but_no_thread_context(int nbMessages)
{
	TestThreadBody__no_thread_context body;
	Threadpool<void, std::string> t( initFunction, body, finalFunction, nullptr, 5, nbMessages);
}

static void executeThreadPool__with_thread_context(int nbMessages)
{
	TestThreadBody__with_thread_context body;
	Threadpool<std::string, std::string> t(Threadpool<void, std::string>::doNothing, body, Threadpool<void, std::string>::doNothing, &TestThreadBody__with_thread_context::reference_context, 5, nbMessages);
	for (int i = 0; i < nbMessages; i++)
		t.add(TestThreadBody__with_thread_context::reference_message);
}



int main()
{
	const int nbMessages = 100000;
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
	executeThreadPool__with_thread_context(nbMessages);
	std::cout << "Execute threadpool with thread context: ";
	if (TestThreadBody__with_thread_context::nbCalls.load() == nbMessages)
		std::cout << "OK" << std::endl;
	else
		std::cout << "NOK" << std::endl;
}
