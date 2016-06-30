#pragma once

namespace cse
{
	namespace serialization
	{
		class PluginCosaveManager
		{
		public:
			class IEventHandler
			{
			public:
				virtual ~IEventHandler() = 0
				{
					;//
				}

				virtual void					HandleLoad(const char* PluginName, const char* CosaveDirectory) = 0;		// plugin name without extension, only called when there's an active plugin
				virtual void					HandleSave(const char* PluginName, const char* CosaveDirectory) = 0;
				virtual void					HandleShutdown(const char* PluginName, const char* CosaveDirectory) = 0;	// plugin name without extension, only called when there's an active plugin
			};
		private:
			static const bgsee::ResourceLocation		kRepositoryPath;

			class GlobalEventSink : public SME::MiscGunk::IEventSink
			{
				PluginCosaveManager*			Parent;
			public:
				GlobalEventSink(PluginCosaveManager* Parent);

				virtual void					Handle(SME::MiscGunk::IEventData* Data);
			};

			friend class GlobalEventSink;
			typedef std::vector<IEventHandler*>			HandlerArrayT;

			HandlerArrayT						Handlers;
			GlobalEventSink*					EventSink;
			bool								Initialized;
			mutable bool						InvokingHandlers;

			void								CleanupEmptyDirectories() const;
			int									GetFileCount(const boost::filesystem::path& DirectoryPath) const;					// includes subdirectories
			bool								CreateCosaveDirectory(TESFile* File) const;
			bool								GetCosaveDirectory(TESFile* File, std::string& OutPath, bool Create = true) const;
			void								HandleLoad() const;
			void								HandleSave() const;
			void								HandleShutdown() const;
		public:
			PluginCosaveManager();
			~PluginCosaveManager();

			void								Initialize();
			void								Deinitialize();
			void								Register(IEventHandler* Handler);
			void								Unregister(IEventHandler* Handler);

			static PluginCosaveManager			Instance;
		};

#define _COSAVE			serialization::PluginCosaveManager::Instance

		template <typename T>
		class TextSerializer
		{
		public:
			virtual ~TextSerializer() = 0
			{
				;//
			}

			virtual bool			Serialize(const T* In, std::string& Out) = 0;	// returns true if successful, false otherwise
			virtual bool			Deserialize(std::string In, T** Out) = 0;		// returns true on success
		};

		class TESForm2Text : public TextSerializer<TESForm>
		{
			static const char*		kSigil;
			static const char*		kNullEditorID;
		public:
			virtual ~TESForm2Text();

			virtual bool			Serialize(const TESForm* In, std::string& Out);
			virtual bool			Deserialize(std::string In, TESForm** Out);
		};

		void Initialize();
		void Deinitialize();
	}
}
