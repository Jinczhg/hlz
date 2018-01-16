#include "Future.h"
//#include <future>

#include <iostream>

#include <unistd.h>
#include <string>

int main(int argc, char** argv)
{
	std::string str = "hello world";
	std::cout << str << std::endl;
	
	ara::com::Promise<int> p;
	p.set_future_dtor_handler([]{std::cout << "future_dtor_handler" << std::endl;});
	ara::com::Future<int> f = p.get_future();
	
	//std::promise<int> p;
	//std::future<int> f = p.get_future();

#if 0	
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
#endif	
	std::thread( [&p]{
	
	 int value = 100;
	 usleep(1000000);
	 p.set_value(value);
	  
	 }).detach();
	
	std::cout << "is ready:" << (f.is_ready() ? "OK" : "NOT") << std::endl;
	
	//f.wait();
	
#if 0
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
#endif
	std::cout<< f.get() << std::endl;
	//std::cout << "is ready:" << (f.is_ready() ? "OK" : "NOT") << std::endl;
	//usleep(100);
	

	return 0;
}
