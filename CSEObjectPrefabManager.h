#pragma once
#include "CSEWrappers.h"

namespace ConstructionSetExtender
{
	namespace ObjectPrefabs
	{
		class CSEObjectPrefabManager;

		// all forms are temporary
		struct PrefabObjectPreviewData
		{
			TESPreviewControl*		Parent;

			TESFormListT			BaseForms;
			TESObjectREFRListT		References;
			NiNode*					RootNode;

			PrefabObjectPreviewData();
			~PrefabObjectPreviewData();

			void					Attach(TESPreviewControl* To);
			void					Detach();
		};

		class PrefabObject
		{
			friend class CSEObjectPrefabManager;

			enum
			{
				kState_None = 0,		// operation not performed (Serialize/Deserialize)
				kState_Good,			// operation completed successfully
				kState_Bad,				// errors encountered during operation/operation couldn't be performed
			};

			CSEPluginFileWrapper*						SourceFile;
			CSEObjectRefCollectionSerializer*			Serializer;
			CSEObjectRefCollectionInstantiator*			Instantiator;

			UInt8										SerializationState;
			UInt8										DeserializationState;
			std::string									FileName;

			void										SetInDialog(HWND Dialog);

			UInt8										Deserialize(bool Force = false);				// returns the state
			UInt8										Serialize(BGSEditorExtender::BGSEEFormListT& Forms, bool Force = false);
		public:
			PrefabObject(const char* FilePath);
			~PrefabObject();

			PrefabObjectPreviewData*					GeneratePreviewData(TESPreviewControl* PreviewControl);		// caller takes ownership of pointer
			bool										Instantiate();
		};

		typedef boost::shared_ptr<PrefabObject>			PrefabObjectHandleT;

		class CSEObjectPrefabManager
		{
			static INT_PTR CALLBACK				DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

			typedef std::vector<PrefabObjectHandleT>			PrefabObjectListT;

			static const BGSEditorExtender::BGSEEResourceLocation		kRepositoryPath;
			static const char*			kPrefabFileExtension;

			PrefabObjectListT			LoadedPrefabs;

			PrefabObject*				CurrentSelection;
			PrefabObjectPreviewData*	PreviewData;

			HWND						MainDialog;
			UInt32						TimeCounter;
			BaseExtraList*				ExtraDataList;
			TESPreviewControl*			Renderer;
			bool						RefreshingList;

			void						InitializeDialog(HWND Dialog);
			void						DeinitializeDialog(HWND Dialog);

			void						RefreshPrefabList();
			void						LoadPrefabsInDirectory(const char* DirectoryPath);

			void						ReloadPrefabs();
			void						NewPrefab();
			void						InstatiateSelection();

			void						UpdatePreview();

			bool						ShowFileDialog(bool Save, std::string& OutPath, std::string& OutName);
		public:
			CSEObjectPrefabManager();
			~CSEObjectPrefabManager();

			void					Show();
			void					Close();

			static CSEObjectPrefabManager			Instance;
		};
	}
}