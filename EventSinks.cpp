#include "EventSinks.h"
#include "ObjectPrefabManager.h"
#include "ObjectPaletteManager.h"
#include "DialogImposterManager.h"
#include "HallOfFame.h"
#include "Render Window\RenderWindowManager.h"
#include "FormUndoStack.h"
#include "[Common]\CLIWrapper.h"

namespace cse
{
	namespace events
	{
		DEFINE_BASIC_EVENT_SINK(DestroyCustomDialogs);
		DEFINE_BASIC_EVENT_SINK_HANDLER(DestroyCustomDialogs)
		{
			PreviewWindowImposterManager::Instance.DestroyImposters();
			objectPalette::ObjectPaletteManager::Instance.Close();
			objectPrefabs::ObjectPrefabManager::Instance.Close();
		}

		DEFINE_BASIC_EVENT_SINK(DataHandlerClearData);
		DEFINE_BASIC_EVENT_SINK_HANDLER(DataHandlerClearData)
		{
			hallOfFame::Deinitialize();
			_RENDERWIN_MGR.GetGroupManager()->Clear();
			GameSettingCollection::Instance->ResetCollection();
			BGSEEUNDOSTACK->Reset();
		}

		DEFINE_BASIC_EVENT_SINK(PostPluginSaveTasks);
		DEFINE_BASIC_EVENT_SINK_HANDLER(PostPluginSaveTasks)
		{
			cliWrapper::interfaces::SE->UpdateIntelliSenseDatabase();
		}

		DEFINE_BASIC_EVENT_SINK(PrePluginLoadTasks);
		DEFINE_BASIC_EVENT_SINK_HANDLER(PrePluginLoadTasks)
		{
			cliWrapper::interfaces::SE->CloseAllOpenEditors();
			cliWrapper::interfaces::USE->HideUseInfoListDialog();
			cliWrapper::interfaces::TAG->HideTagBrowserDialog();
		}

		DEFINE_BASIC_EVENT_SINK(PostPluginLoadTasks);
		DEFINE_BASIC_EVENT_SINK_HANDLER(PostPluginLoadTasks)
		{
			_DATAHANDLER->PerformPostLoadTasks();
			cliWrapper::interfaces::SE->UpdateIntelliSenseDatabase();
		}

		DEFINE_BASIC_EVENT_SINK(DataHandlerConstructSpecialForms);
		DEFINE_BASIC_EVENT_SINK_HANDLER(DataHandlerConstructSpecialForms)
		{
			hallOfFame::Initialize();
		}

		void InitializeSinks()
		{
			ADD_BASIC_SINK_TO_SOURCE(DestroyCustomDialogs, dialog::kCloseAll);
			ADD_BASIC_SINK_TO_SOURCE(DataHandlerClearData, plugin::kClearData);
			ADD_BASIC_SINK_TO_SOURCE(PostPluginSaveTasks, plugin::kPostSave);
			ADD_BASIC_SINK_TO_SOURCE(PrePluginLoadTasks, plugin::kPreLoad);
			ADD_BASIC_SINK_TO_SOURCE(PostPluginLoadTasks, plugin::kPostLoad);
			ADD_BASIC_SINK_TO_SOURCE(DataHandlerConstructSpecialForms, plugin::kConstructSpecialForms);
		}

		void DeinitializeSinks()
		{
			REMOVE_BASIC_SINK_FROM_SOURCE(DestroyCustomDialogs, dialog::kCloseAll);
			REMOVE_BASIC_SINK_FROM_SOURCE(DataHandlerClearData, plugin::kClearData);
			REMOVE_BASIC_SINK_FROM_SOURCE(PostPluginSaveTasks, plugin::kPostSave);
			REMOVE_BASIC_SINK_FROM_SOURCE(PrePluginLoadTasks, plugin::kPreLoad);
			REMOVE_BASIC_SINK_FROM_SOURCE(PostPluginLoadTasks, plugin::kPostLoad);
			REMOVE_BASIC_SINK_FROM_SOURCE(DataHandlerConstructSpecialForms, plugin::kConstructSpecialForms);
		}

	}
}