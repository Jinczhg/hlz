/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-11
 * Author: ryan
 */

#ifndef ARA_COM_FUTURE_H_
#define ARA_COM_FUTURE_H_

#include <stdint.h>
#include <chrono>
#include <future>

namespace ara {
	namespace com {
		enum class FutureStatus : uint8_t {
			ready,
			timeout
		};

		template<class T>
		class Future : protected std::future<T>
		{
			// Default constructor
			Future() noexcept
				: std::future()
			{
			}

			// Move constructor
			Future(Future&& f) noexcept
				: std::future(std::forward<Future>(f))
			{
			}

			// Default copy constructor deleted
			Future(const Future& f) = delete;

			// Specialized unwrapping constructor
			Future(Future<Future<T>> &&) noexcept
			{
				//TODO
			}
			
			~Future()
			{
			}

			// Move assignment operator
			Future& operator=(Future&& f) noexcept
			{
				std::future=(std::forward<Future>(f));
				return *this;
			}

			// Default copy assignment operator deleted
			Future& operator=(const Future&) = delete;

			// Returns the result
			T get()
			{
				return std::future::get();
			}

			// Check if the Future has any shared state
			bool valid() const noexcept
			{
				return std::future::valid();
			}

			// Block until the shared state is ready
			void wait() const
			{
				std::future::wait();
			}

			// Wait for a specified relative time
			template <class Rep, class Period>
			FutureStatus wait_for(const std::chrono::duration<Rep,Period>& timeout_duration) const
			{
				std::future_status status = std::future::wait_for(timeout_duration);
				if (status == std::future_status::ready)
				{
					return FutureStatus::ready;
				}

				return FutureStatus::timeout;
			}

			// Wait until a specified absolute time
			template <class Clock, class Duration>
			FutureStatus wait_until(const std::chrono::time_point<Clock,Duration>& abs_time) const
			{
				std::future_status status = std::future::wait_until(abs_time);
				if (status == std::future_status::ready)
				{
					return FurureStatus::ready;
				}

				return FutureStatus::timeout;
			}

			// Set a continuation for when the shared state is ready
			template <class F>
			auto then(F&& func) -> Future<decltype(func(std::move(*this)))>
			{
				// TODO
				return Future<>;
			}

			// Return true only when the shared state is ready
			bool is_ready() const
			{
				std::future_status status = std::future::wait_for(std::chrono::seconds(0));
				return (status == std::future_status::ready);
			}
		};
	}
}

#endif //ARA_COM_FUTURE_H
