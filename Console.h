#pragma once
#include <bgsee\Console.h>

namespace cse
{
	namespace console
	{
		class ConsoleWarningRegistrar : public bgsee::ConsoleWarningRegistrar
		{
		public:
			virtual ~ConsoleWarningRegistrar();

			virtual void						operator()(bgsee::ConsoleWarningManager* Manager);
		}; 

		void Initialize(void);

		extern bool		Initialized;
	}
}