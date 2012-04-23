#pragma once

// BGSEEWorkspaceManager - Editor workspace manager

namespace BGSEditorExtender
{
	// All paths are absolute unless stated otherwise
	class BGSEEWorkspaceManager
	{
	public:
		struct ReloadPluginsFunctor
		{
			virtual void					operator()(const char* WorkspacePath, bool ResetPluginList, bool LoadESPs) = 0;
		};

		struct DefaultDirectoryDescriptor
		{
			const char**					DirectoryPaths;		// relative to the root game directory
			UInt32							Count;
		};
	private:
		static BGSEEWorkspaceManager*		Singleton;

		BGSEEWorkspaceManager();

		typedef std::list<std::string>		DirectoryListT;

		std::string							CurrentDirectory;
		std::string							DefaultDirectory;
		DirectoryListT						DefaultDirectories;

		VoidRFunctorBase*					WorkspaceResetter;
		ReloadPluginsFunctor*				WorkspacePluginReloader;
		bool								Initialized;

		void								SetWorkingDirectory(const char* WorkspacePath);
		void								CreateDefaultDirectories(const char* WorkspacePath);
	public:
		~BGSEEWorkspaceManager();

		static BGSEEWorkspaceManager*		GetSingleton();

		bool								Initialize(const char* DefaultDirectory,
													VoidRFunctorBase* Resetter,
													ReloadPluginsFunctor* Reloader,
													const DefaultDirectoryDescriptor* DefaultDirectoryData);	// manager takes ownership of the functors

		bool								SelectCurrentWorkspace(const char* DefaultWorkspacePath = NULL);
		const char*							GetCurrentWorkspace(void) const;
	};

	// Always relative to kBasePath
	class BGSEEResourceLocation
	{
		static const std::string			kBasePath;

		std::string							RelativePath;

		bool								CheckPath(void);
	public:
		BGSEEResourceLocation();
		BGSEEResourceLocation(std::string Path);
		~BGSEEResourceLocation();

		BGSEEResourceLocation&				operator=(const BGSEEResourceLocation& rhs);
		BGSEEResourceLocation&				operator=(std::string rhs);
		std::string							operator()();					// returns the full path

		std::string							GetFullPath() const;			// returns BasePath + RelativePath
		std::string							GetRelativePath() const;		// return RelativePath
	};
}