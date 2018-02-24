/*
 * Copyright (c) 2017, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2017-12-14
 * Author: ryan
 */
 
#ifndef ARA_COM_EXCEPTION_H_
#define ARA_COM_EXCEPTION_H_

#include <exception>

namespace ara
{
	namespace com
	{		
		class Exception : public std::exception
		{
		public:
			Exception(std::string what)
			: m_what(what)
			{
			}
			
			virtual ~Exception(){}
			
			virtual const char* what() const noexcept
			{
				return m_what.c_str();
			}
			
		private:
			std::string m_what;
		};
	} // namespace com
} // namespace ara

#endif // ARA_COM_EXCEPTION_H_
