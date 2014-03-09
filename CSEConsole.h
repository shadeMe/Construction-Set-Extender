#pragma once
#include <BGSEEConsole.h>

namespace ConstructionSetExtender
{
	namespace Console
	{
		class CSEConsoleWarningRegistrar : public BGSEditorExtender::BGSEEConsoleWarningRegistrar
		{
		public:
			virtual ~CSEConsoleWarningRegistrar();

			virtual void						operator()(BGSEditorExtender::BGSEEConsoleWarningManager* Manager);
		}; 

		void Initialize(void);

		extern bool		Initialized;
	}
}