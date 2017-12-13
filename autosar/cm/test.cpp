#include "Future.h"
//#include <future>

#include <iostream>

int main(int argc, char** argv)
{
	ara::com::Promise<int> p;
	p.set_future_dtor_handler([]{std::cout << "future_dtor_handler" << std::endl;});
	ara::com::Future<int> f = p.get_future();
	
	//std::promise<int> p;
	//std::future<int> f = p.get_future();
	
	std::thread( [&p]{ p.set_value(100); }).detach();
	
	std::cout << "is ready:" << (f.is_ready() ? "OK" : "NOT") << std::endl;
	
	f.wait();
	
	f.then([](ara::com::Future<int> f){
		try
		{
			std::cout << "then ok" << std::endl;
			std::cout << "Done!\nResults are: "
              << f.get() << std::endl;
		}
		catch(...)
		{
			std::cout << "then error" << std::endl;
		}
	});
	
	//std::cout << "is ready:" << (f.is_ready() ? "OK" : "NOT") << std::endl;

	return 0;
}
