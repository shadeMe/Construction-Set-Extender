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

		WorkspaceManager(const char* DefaultDirectory, WorkspaceManagerOperator* Operator, DefaultDirectoryArrayT& DefaultDirectoryData);
		~WorkspaceManager();

		typedef std::vector<std::string>	DirectoryArrayT;

		std::string							CurrentDirectory;
		std::string							DefaultDirectory;
		DirectoryArrayT						DefaultDirectories;

		WorkspaceManagerOperator*			Operator;
		bool								Initialized;

		void								SetWorkingDirectory(const char* WorkspacePath);
		void								CreateDefaultDirectories(const char* WorkspacePath);
	public:
		static WorkspaceManager*			Get();
		static bool							Initialize(const char* DefaultDirectory,
													WorkspaceManagerOperator* Operator,
													DefaultDirectoryArrayT& DefaultDirectoryData);	// manager takes ownership of the operator
		static void							Deinitialize();

		bool								SelectCurrentWorkspace(const char* DefaultWorkspacePath = nullptr);
		const char*							GetCurrentWorkspace(void) const;
		const char*							GetDefaultWorkspace(void) const;
	};

#define BGSEEWORKSPACE						bgsee::WorkspaceManager::Get()
}