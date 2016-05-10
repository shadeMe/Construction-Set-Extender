#pragma once
#include <BGSEEConsole.h>

namespace ConstructionSetExtender
{
	namespace Console
	{
		class CSEConsoleWarningRegistrar : public bgsee::BGSEEConsoleWarningRegistrar
		{
		public:
			virtual ~CSEConsoleWarningRegistrar();

			virtual void						operator()(bgsee::BGSEEConsoleWarningManager* Manager);
		}; 

		void Initialize(void);

		extern bool		Initialized;
	}
}