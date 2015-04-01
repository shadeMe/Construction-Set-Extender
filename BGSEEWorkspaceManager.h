#pragma once

// BGSEEWorkspaceManager - Editor workspace manager

namespace BGSEditorExtender
{
	class BGSEEWorkspaceManagerOperator
	{
	public:
		virtual ~BGSEEWorkspaceManagerOperator() = 0
		{
			;//
		}

		virtual void						ResetCurrentWorkspace(void) = 0;
		virtual void						ReloadPlugins(const char* WorkspacePath, bool ResetPluginList, bool LoadESPs) = 0;
	};

	// All paths are absolute unless stated otherwise
	class BGSEEWorkspaceManager
	{
	public:
		typedef std::list<std::string>		DefaultDirectoryListT;
	private:
		static BGSEEWorkspaceManager*		Singleton;

		BGSEEWorkspaceManager();

		typedef std::list<std::string>		DirectoryListT;

		std::string							CurrentDirectory;
		std::string							DefaultDirectory;
		DirectoryListT						DefaultDirectories;

		BGSEEWorkspaceManagerOperator*		Operator;
		bool								Initialized;

		void								SetWorkingDirectory(const char* WorkspacePath);
		void								CreateDefaultDirectories(const char* WorkspacePath);
	public:
		~BGSEEWorkspaceManager();

		static BGSEEWorkspaceManager*		GetSingleton();

		bool								Initialize(const char* DefaultDirectory,
													BGSEEWorkspaceManagerOperator* Operator,
													DefaultDirectoryListT& DefaultDirectoryData);	// manager takes ownership of the operator

		bool								SelectCurrentWorkspace(const char* DefaultWorkspacePath = NULL);
		const char*							GetCurrentWorkspace(void) const;
		const char*							GetDefaultWorkspace(void) const;
	};

#define BGSEEWORKSPACE						BGSEditorExtender::BGSEEWorkspaceManager::GetSingleton()

	// Always relative to BasePath, which is Data\BGSEE
	class BGSEEResourceLocation
	{
		std::string							RelativePath;

		bool								CheckPath(void);
	public:
		BGSEEResourceLocation();
		BGSEEResourceLocation(std::string Path);
		~BGSEEResourceLocation();

		BGSEEResourceLocation&				operator=(const BGSEEResourceLocation& rhs);
		BGSEEResourceLocation&				operator=(std::string rhs);
		std::string							operator()() const;				// returns the full path

		std::string							GetFullPath() const;			// returns BasePath + RelativePath
		std::string							GetRelativePath() const;		// return RelativePath

		static const std::string&			GetBasePath(void);
	};
}