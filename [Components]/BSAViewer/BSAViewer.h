#pragma once

namespace cse
{


namespace bsaViewer
{


ref struct ArchiveFile
{
	static enum class eFileType
	{
		Unknown,
		Dds,
		Nif,
		Mp3,
		Wav,
		Lip,
		Kf,
		Spt,
	};

	static Dictionary<String^, eFileType>^ FileTypeExtensions = gcnew Dictionary<String^, eFileType>(StringComparer::CurrentCultureIgnoreCase);

	property String^ Filename;
	property String^ ParentDirectory;
	property UInt32 SizeInBytes;
	property bool Compressed;
	property eFileType FileType;

	ArchiveFile(String^ ParentDirectory, UInt32 SizeInBytes, bool Compressed);

	property String^ FullPath
	{
		String^ get() { return ParentDirectory + "\\" + Filename; }
	}

	property String^ Extension
	{
		String^ get();
	}

	static eFileType GetFileTypeFromExtension(String^ Extension);
};


ref struct DirectoryTreeNode;


ref struct Archive
{
	[Flags]
	static enum class eArchiveFlags : UInt32
	{
		None = 0,
		HasFolderStringTable = 1 << 1,
		HasFileStringTable = 1 << 2,
		Compressed = 1 << 3,
	};

	[Flags]
	static enum class eContentFlags : UInt32
	{
		None = 0,
		Meshes = 1 << 1,
		Textures = 1 << 2,
		Menus = 1 << 3,
		Sounds = 1 << 4,
		Voices = 1 << 5,
		Shader = 1 << 6,
		Trees = 1 << 7,
		Fonts = 1 << 8,
		Misc = 1 << 9,
	};

	property String^ FilePath;
	property bool Valid;
	property eArchiveFlags ArchiveFlags;
	property eContentFlags ContentFlags;
	property List<ArchiveFile^>^ Files;

	Archive(String^ FilePath);

	System::Collections::Generic::ICollection<DirectoryTreeNode^>^ GenerateDirectoryTree(String^ PathFilter, ArchiveFile::eFileType FileTypeFilter);	// returns the top-level nodes of the tree
};


ref struct DirectoryTreeNode
{

	property String^ Name;
	property String^ ParentPath;
	property String^ FullPath;
	property Dictionary<String^, DirectoryTreeNode^>^ Children;		// key is the name of node
	property ArchiveFile^ SourceFile;

	DirectoryTreeNode();
	DirectoryTreeNode(String^ FullPath);

	property bool IsFile
	{
		bool get() { return SourceFile != nullptr; }
	}
};

class ArchiveReader
{
	static String^ ReadNullTerminatedString(BinaryReader^ Stream);
	static String^ ReadString(BinaryReader^ Stream, UInt32 Length);
public:
	static Archive^ ParseArchive(String^ ArchiveFilePath);
};


ref class ArchiveBrowser : public DevComponents::DotNetBar::Metro::MetroForm
{
	static Image^ IconFolder = nullptr;
	static Image^ IconFile = nullptr;

	DevComponents::DotNetBar::Bar^ TopToolbar;
	DevComponents::DotNetBar::ButtonItem^ TopToolbarOpenArchive;
	DevComponents::DotNetBar::LabelItem^ TopToolbarCurrentArchiveName;
	BrightIdeasSoftware::TreeListView^ DirectoryTreeListView;
	DevComponents::DotNetBar::ButtonX^ BottomToolbarOk;
	DevComponents::DotNetBar::Bar^ BottomToolbar;
	DevComponents::DotNetBar::TextBoxItem^ BottomToolbarTextboxFilter;
	DevComponents::DotNetBar::ButtonX^ BottomToolbarCancel;
	System::Windows::Forms::Panel^ BottomToolbarContainer;
	BrightIdeasSoftware::OLVColumn^ DirectoryTreeListViewColName;
	BrightIdeasSoftware::OLVColumn^ DirectoryTreeListViewColSize;
	BrightIdeasSoftware::OLVColumn^ DirectoryTreeListViewColType;
	System::ComponentModel::IContainer^ components;

	void DirectoryTreeListView_KeyDown(Object^ Sender, KeyEventArgs^ E);
	void DirectoryTreeListView_KeyPress(Object^ Sender, KeyPressEventArgs^ E);
	void DirectoryTreeListView_ItemActivate(Object^ Sender, EventArgs^ E);

	void TopToolbarOpenArchive_Click(Object^ Sender, EventArgs^ E);
	void BottomToolbarOk_Click(Object^ Sender, EventArgs^ E);
	void BottomToolbarCancel_Click(Object^ Sender, EventArgs^ E);
	void BottomToolbarTextboxFilter_TextChanged(Object^ Sender, EventArgs^ E);
	void BottomToolbarTextboxFilter_KeyDown(Object^ Sender, KeyEventArgs^ E);

	void Dialog_Load(Object^ Sender, EventArgs^ E);

	void InitializeComponent();
	void FinalizeComponents();
	void CompleteSelection();
	bool LoadArchive(String^ ArchiveFilePath);
	void PopulateDirectoryTree(String^ Filter);
	void UpdateFilter(String^ Filter);

	static Object^ DirectoryTreeAspectNameGetter(Object^ RowObject);
	static Object^ DirectoryTreeImageNamesGetter(Object^ RowObject);
	static Object^ DirectoryTreeAspectSizeGetter(Object^ RowObject);
	static String^ DirectoryTreeAspectToStringConverterSize(Object^ Aspect);
	static Object^ DirectoryTreeAspectTypeGetter(Object^ RowObject);
	static String^ DirectoryTreeAspectToStringConverterType(Object^ Aspect);
	static bool DirectoryTreeCanExpandGetter(Object^ RowObject);
	static System::Collections::IEnumerable^ DirectoryTreeChildrenGetter(Object^ RowObject);
	bool DirectoryTreeFilterPredicate(Object^ Model);

	String^ DefaultDirectory;
	ArchiveFile::eFileType FileTypeFilter;
	Archive^ ActiveArchive;
	ICollection<DirectoryTreeNode^>^ DirectoryTreeData;
	BrightIdeasSoftware::ModelFilter^ FilterPredicate;
public:
	property String^ SelectedFilePath;

	ArchiveBrowser(String^ DefaultDirectory, ArchiveFile::eFileType FileTypeFilter);
	~ArchiveBrowser();
};


} // namespace bsaViewer


} // namespace cse
