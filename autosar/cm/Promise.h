/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-11
 * Author: ryan
 */

#ifndef ARA_COM_PROMISE_H_
#define ARA_COM_PROMISE_H_

#include "Future.h"

template <class T>
class Promise : public std::promise<T> {
public:
	// Default constructor
	Promise()
		: std::promise()
	{
	}

	// Default copy construtor deleted
	Promise(const Promise&) = delete;
	
	// Move constructor
	Promise(Promise&& p) noexcept
		: std::promise(forward<Promise>(p))
	{
	}

	~Promise()
	{
	}

	// Default copy assignment operator delete
	Promise& operator=(const Promise&) = delete;
	
	// Move assignment operator
	Promise& operator=(Promise&& p) noexcept
	{
		std::promise(forward<Promise>(p));
		return *this;
	}

	// Return a Future with the same shared state
	Future<T> get_future()
	{
		return Future<T>(std::promise::get_future());
	}

	//Store an exception in the shared state
	void set_exception(std::exception_ptr p);

	// Store a value in the shared state
	void set_value(const T& value);
	void set_value(T&& value);

	//Set a handler to be called, upon future destruction
	void set_future_dtor_handler(std::function<void> handler);
};

#endif // ARA_COM_PROMISE_H_
