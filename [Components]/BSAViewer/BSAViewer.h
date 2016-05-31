#pragma once

namespace cse
{
	ref class FolderObject;

	public ref class FileSystemObject
	{
	public:
		static enum class							_ObjectType
		{
			e_Folder = 0,
			e_File
		};
		String^										Name;
		_ObjectType									ObjectType;
		FolderObject^								Parent;

		FileSystemObject(String^% Name, FolderObject^% Parent, _ObjectType ObjectType) : Name(Name), Parent(Parent), ObjectType(ObjectType) {}

		String^										GetPath(bool WithRoot);
		String^										GetPathWithName(bool WithRoot)
		{
			String^ Path;
			Path = GetPath(WithRoot);
			if (Path == "")							return Name;
			else									return Path + "\\" + Name;
		}
	};

	public ref class FolderObject : public FileSystemObject
	{
	public:
		List<FileSystemObject^>^					SubItems;

		FolderObject(String^% Name, FolderObject^% Parent) : FileSystemObject(Name, Parent, FileSystemObject::_ObjectType::e_Folder), SubItems(gcnew List<FileSystemObject^>()) {}

		static FolderObject^						NullObj = gcnew FolderObject(gcnew String("__NULL"), NullObj);

		static bool operator ==(FolderObject^% LHS, FolderObject^% RHS)
		{
			return !String::Compare(LHS->Name, RHS->Name, true);
		}
		static bool operator !=(FolderObject^% LHS, FolderObject^% RHS)
		{
			return !(LHS == RHS);
		}
	};
#define FSONULL									FolderObject::NullObj

	public ref class FileObject : public FileSystemObject
	{
	public:
		static enum class							_FileType
		{
			e_Unknown = 0,
			e_DDS,
			e_NIF,
			e_MP3,
			e_WAV,
			e_LIP,
			e_KF,
			e_SPT
		};
		_FileType									FileType;

		FileObject(String^% Name, FolderObject^% Parent) : FileSystemObject(Name, Parent, FileSystemObject::_ObjectType::e_File)
		{
			String^ Extension = Name->Substring(Name->IndexOf(".") + 1);

			if		(!String::Compare(Extension, "dds"))		FileType = (_FileType)1;
			else if (!String::Compare(Extension, "nif"))		FileType = (_FileType)2;
			else if (!String::Compare(Extension, "mp3"))		FileType = (_FileType)3;
			else if (!String::Compare(Extension, "wav"))		FileType = (_FileType)4;
			else if (!String::Compare(Extension, "lip"))		FileType = (_FileType)5;
			else if (!String::Compare(Extension, "kf"))			FileType = (_FileType)6;
			else if (!String::Compare(Extension, "spt"))		FileType = (_FileType)7;
			else												FileType = (_FileType)0;
		}
	};

	public ref class BSAViewer
	{
		static BSAViewer^							Singleton = nullptr;
		BSAViewer();

		void										BSABox_Cancel(Object^ Sender, CancelEventArgs^ E);
		void 										ContentList_ItemActivate(Object^ Sender, EventArgs^ E);
		void 										ToolBarOpenArchive_Click(Object^ Sender, EventArgs^ E);
		void 										ToolBarUp_Click(Object^ Sender, EventArgs^ E);
		void 										ToolBarView_Click(Object^ Sender, EventArgs^ E);

		Form^										BSABox;
		ListView^									ContentList;
		TextBox^									LocationBox;
		OpenFileDialog^								OpenDialog;
		ToolStrip^									ToolBar;
		ToolStripSeparator^							SeparatorA;
		ToolStripButton^							ToolBarOpenArchive;
		ToolStripButton^							ToolBarUp;
		ToolStripButton^							ToolBarView;
		ToolStripLabel^								ToolBarArchiveName;
		StatusStrip^								StatusBar;
		ToolStripLabel^								CurrentLocation;

		ImageList^									LargeIcons;
		ImageList^									SmallIcons;
		String^										Filter;
		String^										ReturnPath;
		FolderObject^								WorkingDirectory;

		FolderObject^ 								GetFolderInDirectory(FolderObject^% WorkingDirectory, String^% FolderName);
		void 										OpenArchive(String^% Path);
		void 										ParseBSAFilePath(String^% Path);
		void 										PopulateContentList(FolderObject^% WorkingDirectory);
		void 										SanitizeReturnPath();
		void										Close();
		void										Cleanup();
	public:
		static BSAViewer^%							GetSingleton();
		static FolderObject^						Root = nullptr;

		String^ 									InitializeViewer(String^% DefaultDirectory, String^% Filter);
	};
#define BSAV									BSAViewer::GetSingleton()
}