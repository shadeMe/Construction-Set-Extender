#pragma once

namespace ConstructionSetExtender
{
	namespace ObjectPalette
	{
		// palette data is serialized to INI files
		typedef std::string		SerializedData;

		class CSEObjectPaletteManager;

		class PaletteObject
		{
			friend class CSEObjectPaletteManager;

			std::string			Name;
			TESForm*			BaseObject;

			Vector3				AngleBase;
			Vector3				AngleOffset;

			Vector2				Sink;			// x = base, y = offset
			Vector2				Scale;			// x = base, y = offset

			bool				ConformToSlope;

			void				Serialize(SerializedData& OutData) const;
			void				Deserialize(const SerializedData& Data);

			void				GetFromDialog(HWND Dialog);
			void				SetInDialog(HWND Dialog) const;

			void				Reset();
		public:
			PaletteObject();
			PaletteObject(TESForm* Base);
			PaletteObject(const SerializedData& Data);
			~PaletteObject();

			TESObjectREFR*		Instantiate(const Vector3& Position, bool MarkAsTemporary) const;
		};

		typedef boost::shared_ptr<PaletteObject>	PaletteObjectHandleT;

		class CSEObjectPaletteManager
		{
			static INT_PTR CALLBACK				DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

			typedef std::vector<PaletteObjectHandleT>			PaletteObjectListT;

			static const BGSEditorExtender::BGSEEResourceLocation		kRepositoryPath;
			static const char*		kPaletteFileExtension;

			PaletteObjectListT		LoadedObjects;
			PaletteObjectListT		CurrentSelection;

			TESObjectREFR*			PreviewRef;

			HWND					MainDialog;
			std::string				CurrentPaletteFilename;
			UInt32					TimeCounter;
			BaseExtraList*			ExtraDataList;
			TESPreviewControl*		Renderer;
			PaletteObject*			ActiveObject;
			bool					RefreshingList;

			void					InitializeDialog(HWND Dialog);
			void					DeinitializeDialog(HWND Dialog);

			void					RefreshObjectList();
			void					UpdateSelectionList();

			void					SaveObjects(const PaletteObjectListT& Objects, const char* Path) const;
			bool					LoadObjects(PaletteObjectListT& OutObjects, const char* Path) const;			// returns false if an error was encountered

			void					NewPalette();
			void					SavePalette(bool NewFile);
			void					LoadPalette();
			void					MergePalette();

			void					UpdatePreview();
			void					HandleDragDrop();

			void					EnableControls(bool State) const;
			bool					GetFormTypeAllowed(UInt8 Type) const;
			bool					ShowFileDialog(bool Save, std::string& OutPath, std::string& OutName);
			bool					GetBaseHandle(PaletteObject* Data, PaletteObjectHandleT& Out) const;
		public:
			CSEObjectPaletteManager();
			~CSEObjectPaletteManager();

			bool					PlaceObject(int X, int Y) const;			// render window coords, returns true if successful

			void					Show();
			void					Close();

			static CSEObjectPaletteManager			Instance;
		};
	}
}