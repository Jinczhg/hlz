/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-11
 * Author: ryan
 */

#ifndef ARA_COM_FUTURE_H_
#define ARA_COM_FUTURE_H_

#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <system_error>
#include <exception>
#include <atomic>
#include <bits/functexcept.h>

namespace std
{
	/// Points to a statically-allocated object derived from error_category.
	const error_category&
	future_category() noexcept;
}

namespace ara
{
	namespace com
	{
		/// Error code for futures
		enum class future_errc
		{
			future_already_retrieved = 1,
			promise_already_satisfied,
			no_state,
			broken_promise
		};

		/// Overload for make_error_code.
		inline std::error_code
		make_error_code(future_errc __errc) noexcept
		{ return std::error_code(static_cast<int>(__errc), std::future_category()); }

		/// Overload for make_error_condition.
		inline std::error_condition
		make_error_condition(future_errc __errc) noexcept
		{ return std::error_condition(static_cast<int>(__errc), std::future_category()); }

		/**
		*  @brief Exception type thrown by futures.
		*  @ingroup exceptions
		*/
		class future_error : public std::logic_error
		{
			std::error_code _M_code;

		public:
			explicit future_error(std::error_code __ec)
			: std::logic_error("std::future_error"), _M_code(__ec)
			{ }

			virtual ~future_error() noexcept
			{}

			virtual const char* what() const noexcept
			{
				return std::logic_error::what();
			}

			const std::error_code& code() const noexcept { return _M_code; }
		};

		// Forward declarations.
		template<typename _Res>
		class Future;

		template<typename _Res>
		class shared_future;

		template<typename _Signature>
		class packaged_task;

		template<typename _Res>
		class Promise;

		/// Launch code for futures
		enum class launch
		{
			async = 1,
			deferred = 2
		};

		constexpr launch operator&(launch __x, launch __y)
		{
			return static_cast<launch>(
			static_cast<int>(__x) & static_cast<int>(__y));
		}

		constexpr launch operator|(launch __x, launch __y)
		{
			return static_cast<launch>(
			static_cast<int>(__x) | static_cast<int>(__y));
		}

		constexpr launch operator^(launch __x, launch __y)
		{
			return static_cast<launch>(
			static_cast<int>(__x) ^ static_cast<int>(__y));
		}

		constexpr launch operator~(launch __x)
		{ return static_cast<launch>(~static_cast<int>(__x)); }

		inline launch& operator&=(launch& __x, launch __y)
		{ return __x = __x & __y; }

		inline launch& operator|=(launch& __x, launch __y)
		{ return __x = __x | __y; }

		inline launch& operator^=(launch& __x, launch __y)
		{ return __x = __x ^ __y; }

		/// Status code for futures
		enum class FutureStatus : uint8_t 
		{
			ready,
			timeout
		};

		template<typename _Fn, typename... _Args>
		Future<typename std::result_of<_Fn(_Args...)>::type>
		async(launch __policy, _Fn&& __fn, _Args&&... __args);

		template<typename _FnCheck, typename _Fn, typename... _Args>
		struct __async_sfinae_helper
		{
			typedef Future<typename std::result_of<_Fn(_Args...)>::type> type;
		};

		template<typename _Fn, typename... _Args>
		struct __async_sfinae_helper<launch, _Fn, _Args...>
		{ };

		template<typename _Fn, typename... _Args>
		typename
		__async_sfinae_helper<typename std::decay<_Fn>::type, _Fn, _Args...>::type
		async(_Fn&& __fn, _Args&&... __args);

#if defined(_GLIBCXX_HAS_GTHREADS) && defined(_GLIBCXX_USE_C99_STDINT_TR1) \
  && (ATOMIC_INT_LOCK_FREE > 1)

		/// Base class and enclosing scope.
		struct __future_base
		{
			/// Base class for results.
			struct _Result_base
			{
				std::exception_ptr _M_error;

				_Result_base(const _Result_base&) = delete;
				_Result_base& operator=(const _Result_base&) = delete;

      			// _M_destroy() allows derived classes to control deallocation
      			virtual void _M_destroy() = 0;

				struct _Deleter
				{
					void operator()(_Result_base* __fr) const { __fr->_M_destroy(); }
				};

			protected:
				_Result_base()
				{}
				virtual ~_Result_base()
				{}
			};

			/// Result.
			template<typename _Res>
			struct _Result : _Result_base
			{
			private:
				typedef std::alignment_of<_Res> __a_of;
				typedef std::aligned_storage<sizeof(_Res), __a_of::value>	__align_storage;
				typedef typename __align_storage::type __align_type;

				__align_type _M_storage;
				bool _M_initialized;

			public:
				_Result() noexcept : _M_initialized() { }
	
				~_Result()
				{
					if (_M_initialized)
						_M_value().~_Res();
				}

				// Return lvalue, future will add const or rvalue-reference
				_Res& _M_value() noexcept { return *static_cast<_Res*>(_M_addr()); }

				void _M_set(const _Res& __res)
				{
					::new (_M_addr()) _Res(__res);
					_M_initialized = true;
				}

				void _M_set(_Res&& __res)
				{
					::new (_M_addr()) _Res(std::move(__res));
					_M_initialized = true;
				}

			private:
				void _M_destroy() { delete this; }

				void* _M_addr() noexcept { return static_cast<void*>(&_M_storage); }
			};

			/// A unique_ptr based on the instantiating type.
			template<typename _Res>
			using _Ptr = std::unique_ptr<_Res, _Result_base::_Deleter>;

			/// Result_alloc.
			template<typename _Res, typename _Alloc>
			struct _Result_alloc final : _Result<_Res>, _Alloc
			{
				typedef typename std::allocator_traits<_Alloc>::template
				rebind_alloc<_Result_alloc> __allocator_type;

				explicit _Result_alloc(const _Alloc& __a) : _Result<_Res>(), _Alloc(__a)
				{ }
	
			private:
				void _M_destroy()
				{
					typedef std::allocator_traits<__allocator_type> __traits;
					__allocator_type __a(*this);
					__traits::destroy(__a, this);
					__traits::deallocate(__a, this, 1);
				}
			};

			template<typename _Res, typename _Allocator>
			static _Ptr<_Result_alloc<_Res, _Allocator>> _S_allocate_result(const _Allocator& __a)
			{
				typedef _Result_alloc<_Res, _Allocator>	__result_type;
				typedef std::allocator_traits<typename __result_type::__allocator_type>
				__traits;
				typename __traits::allocator_type __a2(__a);
				__result_type* __p = __traits::allocate(__a2, 1);
				__try
				{
					__traits::construct(__a2, __p, __a);
				}
				__catch(...)
				{
					__traits::deallocate(__a2, __p, 1);
					__throw_exception_again;
				}
				return _Ptr<__result_type>(__p);
			}


			/// Base class for state between a promise and one or more
			/// associated futures.
			class _State_base
			{
				typedef _Ptr<_Result_base> _Ptr_type;

				_Ptr_type				_M_result;
				std::mutex				_M_mutex;
				std::condition_variable	_M_cond;
				std::atomic_flag		_M_retrieved;
				std::once_flag			_M_once;

			public:
				_State_base() noexcept : _M_result(), _M_retrieved(ATOMIC_FLAG_INIT) { }
				_State_base(const _State_base&) = delete;
				_State_base& operator=(const _State_base&) = delete;
				virtual ~_State_base()
				{}

				_Result_base& wait()
				{
					_M_run_deferred();
					std::unique_lock<std::mutex> __lock(_M_mutex);
					_M_cond.wait(__lock, [&] { return _M_ready(); });
					return *_M_result;
				}

				template<typename _Rep, typename _Period>
				FutureStatus wait_for(const std::chrono::duration<_Rep, _Period>& __rel)
				{
					std::unique_lock<std::mutex> __lock(_M_mutex);
					if (_M_cond.wait_for(__lock, __rel, [&] { return _M_ready(); }))
						return FutureStatus::ready;
					return FutureStatus::timeout;
				}

				template<typename _Clock, typename _Duration>
				FutureStatus wait_until(const std::chrono::time_point<_Clock, _Duration>& __abs)
				{
					std::unique_lock<std::mutex> __lock(_M_mutex);
					if (_M_cond.wait_until(__lock, __abs, [&] { return _M_ready(); }))
						return FutureStatus::ready;
					return FutureStatus::timeout;
				}

				bool is_ready() const
				{
					return _M_ready();
				}

		  		void _M_set_result(std::function<_Ptr_type()> __res, bool __ignore_failure = false)
				{
					bool __set = __ignore_failure;
					// all calls to this function are serialized,
					// side-effects of invoking __res only happen once
					std::call_once(_M_once, &_State_base::_M_do_set, this, std::ref(__res), std::ref(__set));
					if (!__set)
					std::__throw_future_error(int(future_errc::promise_already_satisfied));
				}

				void _M_break_promise(_Ptr_type __res)
				{
					if (static_cast<bool>(__res))
					{
						std::error_code __ec(make_error_code(future_errc::broken_promise));
						//__res->_M_error = std::copy_exception(future_error(__ec));
						__res->_M_error = std::make_exception_ptr(future_error(__ec));
						{
							std::lock_guard<std::mutex> __lock(_M_mutex);
							_M_result.swap(__res);
						}
						_M_cond.notify_all();
					}
				}

				// Called when this object is passed to a future.
				void _M_set_retrieved_flag()
				{
					if (_M_retrieved.test_and_set())
						std::__throw_future_error(int(future_errc::future_already_retrieved));
				}

				template<typename _Res, typename _Arg>
				struct _Setter;

				// set lvalues
				template<typename _Res, typename _Arg>
				struct _Setter<_Res, _Arg&>
				{
					// check this is only used by promise<R>::set_value(const R&)
					// or promise<R>::set_value(R&)
					static_assert(std::is_same<_Res, _Arg&>::value  // promise<R&>
						|| std::is_same<const _Res, _Arg>::value,  // promise<R>
						"Invalid specialisation");

					typename Promise<_Res>::_Ptr_type operator()()
					{
						_State_base::_S_check(_M_promise->_M_future);
						_M_promise->_M_storage->_M_set(_M_arg);
						return std::move(_M_promise->_M_storage);
					}

					Promise<_Res>* _M_promise;
					_Arg& _M_arg;
				};

				// set rvalues
				template<typename _Res>
				struct _Setter<_Res, _Res&&>
				{
					typename Promise<_Res>::_Ptr_type operator()()
					{
						_State_base::_S_check(_M_promise->_M_future);
						_M_promise->_M_storage->_M_set(std::move(_M_arg));
						return std::move(_M_promise->_M_storage);
					}

					Promise<_Res>*    _M_promise;
					_Res&             _M_arg;
				};

				struct __exception_ptr_tag { };

				// set exceptions
				template<typename _Res>
				struct _Setter<_Res, __exception_ptr_tag>
				{
					typename Promise<_Res>::_Ptr_type operator()()
					{
						_State_base::_S_check(_M_promise->_M_future);
						_M_promise->_M_storage->_M_error = _M_ex;
						return std::move(_M_promise->_M_storage);
					}

					Promise<_Res>*   _M_promise;
					std::exception_ptr&    _M_ex;
				};

				template<typename _Res, typename _Arg>
				static _Setter<_Res, _Arg&&>
				__setter(Promise<_Res>* __prom, _Arg&& __arg)
				{
					return _Setter<_Res, _Arg&&>{ __prom, __arg };
				}

				template<typename _Res>
				static _Setter<_Res, __exception_ptr_tag>
				__setter(std::exception_ptr& __ex, Promise<_Res>* __prom)
				{
					return _Setter<_Res, __exception_ptr_tag>{ __prom, __ex };
				}

				static _Setter<void, void>
				__setter(Promise<void>* __prom);

				template<typename _Tp>
				static bool
				_S_check(const std::shared_ptr<_Tp>& __p)
				{
					if (!static_cast<bool>(__p))
						std::__throw_future_error((int)future_errc::no_state);
					
					return true;
				}

			private:
				void _M_do_set(std::function<_Ptr_type()>& __f, bool& __set)
				{
					_Ptr_type __res = __f();
					{
						std::lock_guard<std::mutex> __lock(_M_mutex);
						_M_result.swap(__res);
					}
					_M_cond.notify_all();
					__set = true;
				}

				bool _M_ready() const noexcept { return static_cast<bool>(_M_result); }

				// Misnamed: waits for completion of async function.
				virtual void _M_run_deferred() { }
			};

			template<typename _BoundFn, typename = typename _BoundFn::result_type>
			class _Deferred_state;

			class _Async_state_common;

			template<typename _BoundFn, typename = typename _BoundFn::result_type>
			class _Async_state_impl;

			template<typename _Signature>
			class _Task_state;

			template<typename _BoundFn>
			static std::shared_ptr<_State_base>
			_S_make_deferred_state(_BoundFn&& __fn);

			template<typename _BoundFn>
			static std::shared_ptr<_State_base>
			_S_make_async_state(_BoundFn&& __fn);

			template<typename _Res_ptr, typename _Res>
			struct _Task_setter;

			template<typename _Res_ptr, typename _BoundFn>
			class _Task_setter_helper
			{
				typedef typename std::remove_reference<_BoundFn>::type::result_type __res;
			public:
				typedef _Task_setter<_Res_ptr, __res> __type;
			};

			template<typename _Res_ptr, typename _BoundFn>
			static typename _Task_setter_helper<_Res_ptr, _BoundFn>::__type
			_S_task_setter(_Res_ptr& __ptr, _BoundFn&& __call)
			{
				typedef _Task_setter_helper<_Res_ptr, _BoundFn> __helper_type;
				typedef typename __helper_type::__type _Setter;
				return _Setter{ __ptr, std::ref(__call) };
			}
		};

		/// Partial specialization for reference types.
		template<typename _Res>
    	struct __future_base::_Result<_Res&> : __future_base::_Result_base
    	{
			_Result() noexcept : _M_value_ptr() { }

			void _M_set(_Res& __res) noexcept { _M_value_ptr = &__res; }

			_Res& _M_get() noexcept { return *_M_value_ptr; }

			private:
			_Res* _M_value_ptr;

			void _M_destroy() { delete this; }
		};

		/// Explicit specialization for void.
		template<>
		struct __future_base::_Result<void> : __future_base::_Result_base
		{
		private:
			void _M_destroy() { delete this; }
		};


		/// Common implementation for future and shared_future.
		template<typename _Res>
		class __basic_future : public __future_base
		{
		protected:
			typedef std::shared_ptr<_State_base> __state_type;
			typedef __future_base::_Result<_Res>& __result_type;

		private:
			__state_type _M_state;

		public:
			// Disable copying.
			__basic_future(const __basic_future&) = delete;
			__basic_future& operator=(const __basic_future&) = delete;

			bool valid() const noexcept { return static_cast<bool>(_M_state); }

			void wait() const
			{
				_State_base::_S_check(_M_state);
				_M_state->wait();
			}

			template<typename _Rep, typename _Period>
			FutureStatus wait_for(const std::chrono::duration<_Rep, _Period>& __rel) const
			{
				_State_base::_S_check(_M_state);
				return _M_state->wait_for(__rel);
			}

			template<typename _Clock, typename _Duration>
			FutureStatus wait_until(const std::chrono::time_point<_Clock, _Duration>& __abs) const
			{
				_State_base::_S_check(_M_state);
				return _M_state->wait_until(__abs);
			}

			bool is_ready() const
			{
				_State_base::_S_check(_M_state);
				return _M_state->is_ready();
			}

		protected:
			/// Wait for the state to be ready and rethrow any stored exception
			__result_type _M_get_result()
			{
				_State_base::_S_check(_M_state);
				_Result_base& __res = _M_state->wait();
				if (!(__res._M_error == 0))
				std::rethrow_exception(__res._M_error);
				return static_cast<__result_type>(__res);
			}

			void _M_swap(__basic_future& __that) noexcept
			{
				_M_state.swap(__that._M_state);
			}

			// Construction of a future by promise::get_future()
			explicit __basic_future(const __state_type& __state) : _M_state(__state)
			{
				_State_base::_S_check(_M_state);
				_M_state->_M_set_retrieved_flag();
			}

			// Copy construction from a shared_future
			explicit __basic_future(const shared_future<_Res>&) noexcept;

			// Move construction from a shared_future
			explicit __basic_future(shared_future<_Res>&&) noexcept;

			// Move construction from a future
			explicit __basic_future(Future<_Res>&&) noexcept;

			constexpr __basic_future() noexcept : _M_state() { }

			struct _Reset
			{
				explicit _Reset(__basic_future& __fut) noexcept : _M_fut(__fut) { }
				~_Reset() { _M_fut._M_state.reset(); }
				__basic_future& _M_fut;
			};
		};


		/// Primary template for future.
		template<typename _Res>
		class Future : public __basic_future<_Res>
		{
			friend class Promise<_Res>;
			template<typename> friend class packaged_task;
			template<typename _Fn, typename... _Args>
			friend Future<typename std::result_of<_Fn(_Args...)>::type>
			async(launch, _Fn&&, _Args&&...);

			typedef __basic_future<_Res> _Base_type;
			typedef typename _Base_type::__state_type __state_type;
			
			std::function<void()> _M_dtor_handler;

			explicit Future(const __state_type& __state) : _Base_type(__state) { }
			
			void set_dtor_handler(std::function<void()> handler)
			{
				_M_dtor_handler = handler;
			}

		public:
			constexpr Future() noexcept : _Base_type() { }

			/// Move constructor
			Future(Future&& __uf) noexcept : _Base_type(std::move(__uf)) { }
			
			// Specialized unwrapping constructor
			Future(Future<Future<_Res>> &&) noexcept
			{
				//TODO
			}
			
			virtual ~Future()
			{
				if (_M_dtor_handler)
				{
					_M_dtor_handler();
				}
			}

			// Disable copying
			Future(const Future&) = delete;
			Future& operator=(const Future&) = delete;

			Future& operator=(Future&& __fut) noexcept
			{
				Future(std::move(__fut))._M_swap(*this);
				return *this;
			}

			/// Retrieving the value
			_Res get()
			{
				typename _Base_type::_Reset __reset(*this);
				return std::move(this->_M_get_result()._M_value());
			}
			
			// Set a continuation for when the shared state is ready
			template <class F>
			auto then(F&& func) -> Future<decltype(func(std::move(*this)))>
			{
				if (this->is_ready())
				{
					func(std::move(*this));
				}
				
				return Future<decltype(func(std::move(*this)))>();
			}

			shared_future<_Res> share();
		};

		/// Partial specialization for future<R&>
		template<typename _Res>
		class Future<_Res&> : public __basic_future<_Res&>
		{
			friend class Promise<_Res&>;
			template<typename> friend class packaged_task;
			template<typename _Fn, typename... _Args>
			friend Future<typename std::result_of<_Fn(_Args...)>::type>
			async(launch, _Fn&&, _Args&&...);

			typedef __basic_future<_Res&> _Base_type;
			typedef typename _Base_type::__state_type __state_type;
			
			std::function<void()> _M_dtor_handler;

			explicit Future(const __state_type& __state) : _Base_type(__state) { }
			
			void set_dtor_handler(std::function<void()> handler)
			{
				_M_dtor_handler = handler;
			}

		public:
			constexpr Future() noexcept : _Base_type() { }

			/// Move constructor
			Future(Future&& __uf) noexcept : _Base_type(std::move(__uf)) { }
			
			// Specialized unwrapping constructor
			Future(Future<Future<_Res&>> &&) noexcept
			{
				//TODO
			}
			
			virtual ~Future()
			{
				if (_M_dtor_handler)
				{
					_M_dtor_handler();
				}
			}

			// Disable copying
			Future(const Future&) = delete;
			Future& operator=(const Future&) = delete;

			Future& operator=(Future&& __fut) noexcept
			{
				Future(std::move(__fut))._M_swap(*this);
				return *this;
			}

			/// Retrieving the value
			_Res& get()
			{
				typename _Base_type::_Reset __reset(*this);
				return this->_M_get_result()._M_get();
			}
			
			// Set a continuation for when the shared state is ready
			template <class F>
			auto then(F&& func) -> Future<decltype(func(std::move(*this)))>
			{
				if (this->is_ready())
				{
					func(*this);
					return Future<decltype(func(*this))>();
				}
				
				return Future<decltype(func(*this))>();
			}

			shared_future<_Res&> share();
		};

		/// Explicit specialization for future<void>
		template<>
		class Future<void> : public __basic_future<void>
		{
			friend class Promise<void>;
			template<typename> friend class packaged_task;
			template<typename _Fn, typename... _Args>
			friend Future<typename std::result_of<_Fn(_Args...)>::type>
			async(launch, _Fn&&, _Args&&...);

			typedef __basic_future<void> _Base_type;
			typedef typename _Base_type::__state_type __state_type;
			
			std::function<void()> _M_dtor_handler;

			explicit Future(const __state_type& __state) : _Base_type(__state) { }
			
			void set_dtor_handler(std::function<void()> handler)
			{
				_M_dtor_handler = handler;
			}

		public:
			Future() noexcept : _Base_type() { }

			/// Move constructor
			Future(Future&& __uf) noexcept : _Base_type(std::move(__uf)) { }
			
			// Specialized unwrapping constructor
			Future(Future<Future<void>> &&) noexcept
			{
				//TODO
			}
			
			virtual ~Future()
			{
				if (_M_dtor_handler)
				{
					_M_dtor_handler();
				}
			}

			// Disable copying
			Future(const Future&) = delete;
			Future& operator=(const Future&) = delete;

			Future& operator=(Future&& __fut) noexcept
			{
				Future(std::move(__fut))._M_swap(*this);
				return *this;
			}

			/// Retrieving the value
			void get()
			{
				typename _Base_type::_Reset __reset(*this);
				this->_M_get_result();
			}
			
			// Set a continuation for when the shared state is ready
			template <class F>
			auto then(F&& func) -> Future<decltype(func(std::move(*this)))>
			{
				if (this->is_ready())
				{
					func(*this);
					return Future<decltype(func(*this))>();
				}
				
				return Future<decltype(func(*this))>();
			}

			shared_future<void> share();
		};


		/// Primary template for shared_future.
		template<typename _Res>
		class shared_future : public __basic_future<_Res>
		{
			typedef __basic_future<_Res> _Base_type;

		public:
			constexpr shared_future() noexcept : _Base_type() { }

			/// Copy constructor
			shared_future(const shared_future& __sf) : _Base_type(__sf) { }

			/// Construct from a future rvalue
			shared_future(Future<_Res>&& __uf) noexcept
			: _Base_type(std::move(__uf))
			{ }

			/// Construct from a shared_future rvalue
			shared_future(shared_future&& __sf) noexcept
			: _Base_type(std::move(__sf))
			{ }

			shared_future& operator=(const shared_future& __sf)
			{
				shared_future(__sf)._M_swap(*this);
				return *this;
			}

			shared_future& operator=(shared_future&& __sf) noexcept
			{
				shared_future(std::move(__sf))._M_swap(*this);
				return *this;
			}

			/// Retrieving the value
			const _Res& get()
			{
				typename _Base_type::__result_type __r = this->_M_get_result();
				_Res& __rs(__r._M_value());
				return __rs;
			}
		};

		/// Partial specialization for shared_future<R&>
		template<typename _Res>
		class shared_future<_Res&> : public __basic_future<_Res&>
		{
			typedef __basic_future<_Res&>           _Base_type;

		public:
			constexpr shared_future() noexcept : _Base_type() { }

			/// Copy constructor
			shared_future(const shared_future& __sf) : _Base_type(__sf) { }

			/// Construct from a future rvalue
			shared_future(Future<_Res&>&& __uf) noexcept
			: _Base_type(std::move(__uf))
			{ }

			/// Construct from a shared_future rvalue
			shared_future(shared_future&& __sf) noexcept
			: _Base_type(std::move(__sf))
			{ }

			shared_future& operator=(const shared_future& __sf)
			{
				shared_future(__sf)._M_swap(*this);
				return *this;
			}

			shared_future& operator=(shared_future&& __sf) noexcept
			{
				shared_future(std::move(__sf))._M_swap(*this);
 				return *this;
			}

			/// Retrieving the value
			_Res& get()
			{ 
				return this->_M_get_result()._M_get();
			}
		};

		/// Explicit specialization for shared_future<void>
		template<>
		class shared_future<void> : public __basic_future<void>
		{
			typedef __basic_future<void> _Base_type;

		public:
			constexpr shared_future() noexcept : _Base_type() { }

			/// Copy constructor
			shared_future(const shared_future& __sf) : _Base_type(__sf) { }

			/// Construct from a future rvalue
			shared_future(Future<void>&& __uf) noexcept
			: _Base_type(std::move(__uf))
			{ }

			/// Construct from a shared_future rvalue
			shared_future(shared_future&& __sf) noexcept
			: _Base_type(std::move(__sf))
			{ }

			shared_future& operator=(const shared_future& __sf)
			{
				shared_future(__sf)._M_swap(*this);
				return *this;
			}

			shared_future& operator=(shared_future&& __sf) noexcept
			{
				shared_future(std::move(__sf))._M_swap(*this);
				return *this;
			}

			// Retrieving the value
			void get()
			{
				this->_M_get_result();
			}
		};

		// Now we can define the protected __basic_future constructors.
		template<typename _Res>
		inline __basic_future<_Res>::
		__basic_future(const shared_future<_Res>& __sf) noexcept
		: _M_state(__sf._M_state)
		{ }

		template<typename _Res>
		inline __basic_future<_Res>::
		__basic_future(shared_future<_Res>&& __sf) noexcept
		: _M_state(std::move(__sf._M_state))
		{ }

		template<typename _Res>
		inline __basic_future<_Res>::
		__basic_future(Future<_Res>&& __uf) noexcept
		: _M_state(std::move(__uf._M_state))
		{ }

		template<typename _Res>
		inline shared_future<_Res>
		Future<_Res>::share()
		{ return shared_future<_Res>(std::move(*this)); }

		template<typename _Res>
		inline shared_future<_Res&>
		Future<_Res&>::share()
		{ return shared_future<_Res&>(std::move(*this)); }

		inline shared_future<void>
		Future<void>::share()
		{ return shared_future<void>(std::move(*this)); }

		/// Primary template for promise
		template<typename _Res>
		class Promise
		{
			typedef __future_base::_State_base _State;
			typedef __future_base::_Result<_Res> _Res_type;
			typedef __future_base::_Ptr<_Res_type> _Ptr_type;
			template<typename, typename> friend class _State::_Setter;

			std::shared_ptr<_State> _M_future;
			_Ptr_type _M_storage;
			std::function<void()> _M_future_dtor_handler;

		public:
			Promise()
			: _M_future(std::make_shared<_State>()),
			_M_storage(new _Res_type())
			{ }

			Promise(Promise&& __rhs) noexcept
			: _M_future(std::move(__rhs._M_future)),
			_M_storage(std::move(__rhs._M_storage))
			{ }

			template<typename _Allocator>
			Promise(std::allocator_arg_t, const _Allocator& __a)
			: _M_future(std::allocate_shared<_State>(__a)),
			_M_storage(__future_base::_S_allocate_result<_Res>(__a))
			{ }

			template<typename _Allocator>
			Promise(std::allocator_arg_t, const _Allocator&, Promise&& __rhs)
			: _M_future(std::move(__rhs._M_future)),
			_M_storage(std::move(__rhs._M_storage))
			{ }

			Promise(const Promise&) = delete;

			~Promise()
			{
				if (static_cast<bool>(_M_future) && !_M_future.unique())
					_M_future->_M_break_promise(std::move(_M_storage));
			}

			// Assignment
			Promise& operator=(Promise&& __rhs) noexcept
			{
				Promise(std::move(__rhs)).swap(*this);
				return *this;
			}

			Promise& operator=(const Promise&) = delete;

			void swap(Promise& __rhs) noexcept
			{
				_M_future.swap(__rhs._M_future);
				_M_storage.swap(__rhs._M_storage);
			}

			// Retrieving the result
			Future<_Res> get_future()
			{
				//return Future<_Res>(_M_future);
				Future<_Res> f(_M_future);
				f.set_dtor_handler(_M_future_dtor_handler);
				return f;
			}

			// Setting the result
			void set_value(const _Res& __r)
			{
				auto __setter = _State::__setter(this, __r);
				_M_future->_M_set_result(std::move(__setter));
			}

			void set_value(_Res&& __r)
			{
				auto __setter = _State::__setter(this, std::move(__r));
				_M_future->_M_set_result(std::move(__setter));
			}
			
			//Store an exception in the shared state
			void set_exception(std::exception_ptr __p)
			{
				auto __setter = _State::__setter(__p, this);
				_M_future->_M_set_result(std::move(__setter));
			}
			
			//Set a handler to be called, upon future destruction
			void set_future_dtor_handler(std::function<void()> handler)
			{
				_M_future_dtor_handler = handler;
			}
		};

		template<typename _Res>
		inline void swap(Promise<_Res>& __x, Promise<_Res>& __y) noexcept
		{
			__x.swap(__y);
		}


		/// Partial specialization for promise<R&>
		template<typename _Res>
		class Promise<_Res&>
		{
			typedef __future_base::_State_base _State;
			typedef __future_base::_Result<_Res&> _Res_type;
			typedef __future_base::_Ptr<_Res_type> _Ptr_type;
			template<typename, typename> friend class _State::_Setter;

			std::shared_ptr<_State> _M_future;
			_Ptr_type _M_storage;
			std::function<void()> _M_future_dtor_handler;

		public:
			Promise()
			: _M_future(std::make_shared<_State>()),
			_M_storage(new _Res_type())
			{ }

			Promise(Promise&& __rhs) noexcept
			: _M_future(std::move(__rhs._M_future)),
			_M_storage(std::move(__rhs._M_storage))
			{ }

			template<typename _Allocator>
			Promise(std::allocator_arg_t, const _Allocator& __a)
			: _M_future(std::allocate_shared<_State>(__a)),
			_M_storage(__future_base::_S_allocate_result<_Res&>(__a))
			{ }

			template<typename _Allocator>
			Promise(std::allocator_arg_t, const _Allocator&, Promise&& __rhs)
			: _M_future(std::move(__rhs._M_future)),
			_M_storage(std::move(__rhs._M_storage))
			{ }

			Promise(const Promise&) = delete;

			~Promise()
			{
				if (static_cast<bool>(_M_future) && !_M_future.unique())
					_M_future->_M_break_promise(std::move(_M_storage));
			}

			// Assignment
			Promise& operator=(Promise&& __rhs) noexcept
			{
				Promise(std::move(__rhs)).swap(*this);
				return *this;
			}

			Promise& operator=(const Promise&) = delete;

			void swap(Promise& __rhs) noexcept
			{
				_M_future.swap(__rhs._M_future);
				_M_storage.swap(__rhs._M_storage);
			}

			// Retrieving the result
			Future<_Res&> get_future()
			{
				//return Future<_Res&>(_M_future);
				Future<_Res&> f(_M_future);
				f.set_dtor_handler(_M_future_dtor_handler);
				return f;
			}

			// Setting the result
			void set_value(_Res& __r)
			{
				auto __setter = _State::__setter(this, __r);
				_M_future->_M_set_result(std::move(__setter));
			}

			//Store an exception in the shared state
			void set_exception(std::exception_ptr __p)
			{
				auto __setter = _State::__setter(__p, this);
				_M_future->_M_set_result(std::move(__setter));
			}
			
			//Set a handler to be called, upon future destruction
			void set_future_dtor_handler(std::function<void()> handler)
			{
				_M_future_dtor_handler = handler;
			}
		};

		/// Explicit specialization for promise<void>
		template<>
		class Promise<void>
		{
			typedef __future_base::_State_base _State;
			typedef __future_base::_Result<void> _Res_type;
			typedef __future_base::_Ptr<_Res_type> _Ptr_type;
			template<typename, typename> friend class _State::_Setter;

			std::shared_ptr<_State> _M_future;
			_Ptr_type _M_storage;
			std::function<void()> _M_future_dtor_handler;

		public:
			Promise()
			: _M_future(std::make_shared<_State>()),
			_M_storage(new _Res_type())
			{ }

			Promise(Promise&& __rhs) noexcept
			: _M_future(std::move(__rhs._M_future)),
			_M_storage(std::move(__rhs._M_storage))
			{ }

			template<typename _Allocator>
			Promise(std::allocator_arg_t, const _Allocator& __a)
			: _M_future(std::allocate_shared<_State>(__a)),
			_M_storage(__future_base::_S_allocate_result<void>(__a))
			{ }

			template<typename _Allocator>
			Promise(std::allocator_arg_t, const _Allocator&, Promise&& __rhs)
			: _M_future(std::move(__rhs._M_future)),
			_M_storage(std::move(__rhs._M_storage))
			{ }

			Promise(const Promise&) = delete;

			~Promise()
			{
				if (static_cast<bool>(_M_future) && !_M_future.unique())
					_M_future->_M_break_promise(std::move(_M_storage));
			}

			// Assignment
			Promise& operator=(Promise&& __rhs) noexcept
			{
				Promise(std::move(__rhs)).swap(*this);
				return *this;
			}

			Promise& operator=(const Promise&) = delete;

			void swap(Promise& __rhs) noexcept
			{
				_M_future.swap(__rhs._M_future);
				_M_storage.swap(__rhs._M_storage);
			}

			// Retrieving the result
			Future<void> get_future()
			{
				//return Future<void>(_M_future);
				Future<void> f(_M_future);
				f.set_dtor_handler(_M_future_dtor_handler);
				return f;
			}

			// Setting the result
			void set_value();
			
			//Store an exception in the shared state
			void set_exception(std::exception_ptr __p)
			{
				auto __setter = _State::__setter(__p, this);
				_M_future->_M_set_result(std::move(__setter));
			}
			
			//Set a handler to be called, upon future destruction
			void set_future_dtor_handler(std::function<void()> handler)
			{
				_M_future_dtor_handler = handler;
			}
		};

		// set void
		template<>
		struct __future_base::_State_base::_Setter<void, void>
		{
			Promise<void>::_Ptr_type operator()()
			{
				_State_base::_S_check(_M_promise->_M_future);
				return std::move(_M_promise->_M_storage);
			}

			Promise<void>*    _M_promise;
		};

		inline __future_base::_State_base::_Setter<void, void>
		__future_base::_State_base::__setter(Promise<void>* __prom)
		{
			return _Setter<void, void>{ __prom };
		}

		inline void
		Promise<void>::set_value()
		{
			auto __setter = _State::__setter(this);
			_M_future->_M_set_result(std::move(__setter));
		}


		template<typename _Ptr_type, typename _Res>
		struct __future_base::_Task_setter
		{
			_Ptr_type operator()()
			{
				__try
				{
					_M_result->_M_set(_M_fn());
				}
				__catch(...)
				{
					_M_result->_M_error = std::current_exception();
				}
				return std::move(_M_result);
			}

			_Ptr_type& _M_result;
			std::function<_Res()> _M_fn;
		};

		template<typename _Ptr_type>
		struct __future_base::_Task_setter<_Ptr_type, void>
		{
			_Ptr_type operator()()
			{
				__try
				{
					_M_fn();
				}
				__catch(...)
				{
					_M_result->_M_error = std::current_exception();
				}
				return std::move(_M_result);
			}

			_Ptr_type&                _M_result;
			std::function<void()>     _M_fn;
		};

		template<typename _Res, typename... _Args>
		struct __future_base::_Task_state<_Res(_Args...)> final
		: __future_base::_State_base
		{
			typedef _Res _Res_type;

			_Task_state(std::function<_Res(_Args...)> __task)
			: _M_result(new _Result<_Res>()), _M_task(std::move(__task))
			{ }

			template<typename _Func, typename _Alloc>
			_Task_state(_Func&& __task, const _Alloc& __a)
			: _M_result(_S_allocate_result<_Res>(__a)),
			_M_task(std::allocator_arg, __a, std::move(__task))
			{ }

			void _M_run(_Args... __args)
			{
				// bound arguments decay so wrap lvalue references
				auto __boundfn = std::__bind_simple(std::ref(_M_task),
				_S_maybe_wrap_ref(std::forward<_Args>(__args))...);
				auto __setter = _S_task_setter(_M_result, std::move(__boundfn));
				_M_set_result(std::move(__setter));
			}

			typedef __future_base::_Ptr<_Result<_Res>> _Ptr_type;
			_Ptr_type _M_result;
			std::function<_Res(_Args...)> _M_task;

			template<typename _Tp>
			static std::reference_wrapper<_Tp> _S_maybe_wrap_ref(_Tp& __t)
			{
				return std::ref(__t);
			}

			template<typename _Tp>
			static typename std::enable_if<!std::is_lvalue_reference<_Tp>::value, _Tp>::type&&
			_S_maybe_wrap_ref(_Tp&& __t)
			{
				return std::forward<_Tp>(__t);
			}
		};

		template<typename _Task, typename _Fn, bool
			= std::is_same<_Task, typename std::decay<_Fn>::type>::value>
		struct __constrain_pkgdtask
		{
			typedef void __type;
		};

		template<typename _Task, typename _Fn>
		struct __constrain_pkgdtask<_Task, _Fn, true>
		{ };

		/// packaged_task
		template<typename _Res, typename... _ArgTypes>
		class packaged_task<_Res(_ArgTypes...)>
		{
			typedef __future_base::_Task_state<_Res(_ArgTypes...)> _State_type;
			std::shared_ptr<_State_type> _M_state;

		public:
			// Construction and destruction
			packaged_task() noexcept { }

			template<typename _Allocator>
			explicit packaged_task(std::allocator_arg_t, const _Allocator& __a) noexcept
			{ }

			template<typename _Fn, typename = typename
				__constrain_pkgdtask<packaged_task, _Fn>::__type>
			explicit packaged_task(_Fn&& __fn)
			: _M_state(std::make_shared<_State_type>(std::forward<_Fn>(__fn)))
			{ }

			template<typename _Fn, typename _Allocator, typename = typename
				__constrain_pkgdtask<packaged_task, _Fn>::__type>
			explicit packaged_task(std::allocator_arg_t, const _Allocator& __a, _Fn&& __fn)
			: _M_state(std::allocate_shared<_State_type>(__a, std::forward<_Fn>(__fn)))
			{ }

			~packaged_task()
			{
				if (static_cast<bool>(_M_state) && !_M_state.unique())
					_M_state->_M_break_promise(std::move(_M_state->_M_result));
			}

			// No copy
			packaged_task(const packaged_task&) = delete;
			packaged_task& operator=(const packaged_task&) = delete;

			template<typename _Allocator>
			explicit packaged_task(std::allocator_arg_t, const _Allocator&, const packaged_task&) = delete;

			// Move support
			packaged_task(packaged_task&& __other) noexcept
			{ this->swap(__other); }

			template<typename _Allocator>
			explicit packaged_task(std::allocator_arg_t, const _Allocator&,
				packaged_task&& __other) noexcept
			{ this->swap(__other); }

			packaged_task& operator=(packaged_task&& __other) noexcept
			{
				packaged_task(std::move(__other)).swap(*this);
				return *this;
			}

			void swap(packaged_task& __other) noexcept
			{ _M_state.swap(__other._M_state); }

			bool valid() const noexcept
			{ return static_cast<bool>(_M_state); }

			// Result retrieval
			Future<_Res> get_future()
			{
				return Future<_Res>(_M_state);
			}

			// Execution
			void operator()(_ArgTypes... __args)
			{
				__future_base::_State_base::_S_check(_M_state);
				_M_state->_M_run(std::forward<_ArgTypes>(__args)...);
			}

			void reset()
			{
				__future_base::_State_base::_S_check(_M_state);
				packaged_task(std::move(_M_state->_M_task)).swap(*this);
			}
		};

		/// swap
		template<typename _Res, typename... _ArgTypes>
		inline void swap(packaged_task<_Res(_ArgTypes...)>& __x, packaged_task<_Res(_ArgTypes...)>& __y) noexcept
		{ __x.swap(__y); }

		template<typename _BoundFn, typename _Res>
		class __future_base::_Deferred_state final : public __future_base::_State_base
		{
		public:
			explicit _Deferred_state(_BoundFn&& __fn)
			: _M_result(new _Result<_Res>()), _M_fn(std::move(__fn))
			{ }

		private:
			typedef __future_base::_Ptr<_Result<_Res>> _Ptr_type;
			_Ptr_type _M_result;
			_BoundFn _M_fn;

			virtual void _M_run_deferred()
			{
				// safe to call multiple times so ignore failure
				_M_set_result(_S_task_setter(_M_result, _M_fn), true);
			}
		};

		class __future_base::_Async_state_common : public __future_base::_State_base
		{
		protected:
#ifdef _GLIBCXX_ASYNC_ABI_COMPAT
			~_Async_state_common();
#else
			~_Async_state_common() = default;
#endif

			// Allow non-timed waiting functions to block until the thread completes,
			// as if joined.
			virtual void _M_run_deferred() { _M_join(); }

			void _M_join() { std::call_once(_M_once, &std::thread::join, ref(_M_thread)); }

			std::thread _M_thread;
			std::once_flag _M_once;
		};

		template<typename _BoundFn, typename _Res>
		class __future_base::_Async_state_impl final
		: public __future_base::_Async_state_common
		{
		public:
			explicit _Async_state_impl(_BoundFn&& __fn)
			: _M_result(new _Result<_Res>()), _M_fn(std::move(__fn))
			{
				_M_thread = std::thread{ [this] {
					_M_set_result(_S_task_setter(_M_result, _M_fn));
				} };
      		}

			~_Async_state_impl() { _M_join(); }

		private:
			typedef __future_base::_Ptr<_Result<_Res>> _Ptr_type;
			_Ptr_type _M_result;
			_BoundFn _M_fn;
		};

		template<typename _BoundFn>
		inline std::shared_ptr<__future_base::_State_base>
		__future_base::_S_make_deferred_state(_BoundFn&& __fn)
		{
			typedef typename std::remove_reference<_BoundFn>::type __fn_type;
			typedef _Deferred_state<__fn_type> __state_type;
			return std::make_shared<__state_type>(std::move(__fn));
		}

		template<typename _BoundFn>
		inline std::shared_ptr<__future_base::_State_base>
		__future_base::_S_make_async_state(_BoundFn&& __fn)
		{
			typedef typename std::remove_reference<_BoundFn>::type __fn_type;
			typedef _Async_state_impl<__fn_type> __state_type;
			return std::make_shared<__state_type>(std::move(__fn));
		}


		/// async
		template<typename _Fn, typename... _Args>
		Future<typename std::result_of<_Fn(_Args...)>::type>
		async(launch __policy, _Fn&& __fn, _Args&&... __args)
		{
			typedef typename std::result_of<_Fn(_Args...)>::type result_type;
			std::shared_ptr<__future_base::_State_base> __state;
			if ((__policy & (launch::async|launch::deferred)) == launch::async)
			{
				__state = __future_base::_S_make_async_state(std::__bind_simple(
				std::forward<_Fn>(__fn), std::forward<_Args>(__args)...));
			}
			else
			{
				__state = __future_base::_S_make_deferred_state(std::__bind_simple(
				std::forward<_Fn>(__fn), std::forward<_Args>(__args)...));
			}
			return Future<result_type>(__state);
		}

		/// async, potential overload
		template<typename _Fn, typename... _Args>
		inline typename
		__async_sfinae_helper<typename std::decay<_Fn>::type, _Fn, _Args...>::type
		async(_Fn&& __fn, _Args&&... __args)
		{
			return async(launch::async|launch::deferred, std::forward<_Fn>(__fn),
			std::forward<_Args>(__args)...);
		}

#endif // _GLIBCXX_HAS_GTHREADS && _GLIBCXX_USE_C99_STDINT_TR1
       // && ATOMIC_INT_LOCK_FREE

	} // namespace com
} // namespace ara

namespace std
{
	/// Specialization.
	template<>
	struct is_error_code_enum<ara::com::future_errc> : public true_type { };
	
	template<typename _Res, typename _Alloc>
	struct uses_allocator<ara::com::Promise<_Res>, _Alloc>
	: public true_type { };
	
	template<typename _Res, typename _Alloc>
	struct uses_allocator<ara::com::packaged_task<_Res>, _Alloc>
	: public true_type { };
} //namespace

#endif // ARA_COM_FUTURE_H_
