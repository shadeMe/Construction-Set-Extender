#pragma once

// WorkspaceManager - Editor workspace manager

namespace bgsee
{
	class WorkspaceManagerOperator
	{
	public:
		virtual ~WorkspaceManagerOperator() = 0
		{
			;//
		}

		virtual void						ResetCurrentWorkspace(void) = 0;
		virtual void						ReloadPlugins(const char* WorkspacePath, bool ResetPluginList, bool LoadESPs) = 0;
	};

	// All paths are absolute unless stated otherwise
	class WorkspaceManager
	{
	public:
		typedef std::vector<std::string>	DefaultDirectoryArrayT;
	private:
		static WorkspaceManager*		Singleton;

		WorkspaceManager();

		typedef std::vector<std::string>	DirectoryArrayT;

		std::string							CurrentDirectory;
		std::string							DefaultDirectory;
		DirectoryArrayT						DefaultDirectories;

		WorkspaceManagerOperator*			Operator;
		bool								Initialized;

		void								SetWorkingDirectory(const char* WorkspacePath);
		void								CreateDefaultDirectories(const char* WorkspacePath);
	public:
		~WorkspaceManager();

		static WorkspaceManager*			GetSingleton();

		bool								Initialize(const char* DefaultDirectory,
													WorkspaceManagerOperator* Operator,
													DefaultDirectoryArrayT& DefaultDirectoryData);	// manager takes ownership of the operator

		bool								SelectCurrentWorkspace(const char* DefaultWorkspacePath = nullptr);
		const char*							GetCurrentWorkspace(void) const;
		const char*							GetDefaultWorkspace(void) const;
	};

#define BGSEEWORKSPACE						bgsee::WorkspaceManager::GetSingleton()

	// Always relative to BasePath, which is Data\BGSEE
	class ResourceLocation
	{
		std::string							RelativePath;

		bool								CheckPath(void);
	public:
		ResourceLocation();
		ResourceLocation(std::string Path);
		~ResourceLocation();

		ResourceLocation&					operator=(const ResourceLocation& rhs);
		ResourceLocation&					operator=(std::string rhs);
		std::string							operator()() const;				// returns the full path

		std::string							GetFullPath() const;			// returns BasePath + RelativePath
		std::string							GetRelativePath() const;		// return RelativePath

		static const std::string&			GetBasePath(void);
	};
}