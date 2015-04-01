#pragma once

#include <BGSEEWrappers.h>

namespace ConstructionSetExtender
{
	class CSEFormWrapper : public BGSEditorExtender::BGSEEFormWrapper
	{
	protected:
		TESForm*							WrappedForm;
	public:
		CSEFormWrapper(TESForm* Form);
		virtual ~CSEFormWrapper();

		virtual UInt32						GetFormID(void) const;
		virtual const char*					GetEditorID(void) const;
		virtual UInt8						GetType(void) const;
		virtual const char*					GetTypeString(void) const;
		virtual UInt32						GetFlags(void) const;

		virtual bool						GetIsDeleted(void) const;

		TESForm*							GetWrappedForm(void) const;
	};

	class CSEPluginFileWrapper : public BGSEditorExtender::BGSEEPluginFileWrapper
	{
	protected:
		TESFile*							WrappedPlugin;
		std::string							PluginPath;

		void								CreateTempFile(void);
	public:
		CSEPluginFileWrapper();
		virtual ~CSEPluginFileWrapper();

		virtual bool						Construct(const char* FileName);
		virtual void						Purge(void);

		virtual bool						Open(bool ForWriting);
		virtual bool						SaveHeader(void);
		virtual bool						CorrectHeader(UInt32 RecordCount);
		virtual bool						Close(void);

		virtual UInt8						GetRecordType(void);
		virtual bool						GetNextRecord(bool SkipIgnoredRecords);

		virtual int							GetErrorState(void) const;
		virtual const char*					GetFileName(void) const;

		TESFile*							GetWrappedPlugin(void) const;
	};

	class ICSEFormCollectionSerializer : public BGSEditorExtender::BGSEEFormCollectionSerializer
	{
	protected:
		TESFormListT					LoadedFormBuffer;		// stores loaded forms for deferred linking

		void							FreeBuffer(void);

		virtual bool					LoadForm(BGSEditorExtender::BGSEEPluginFileWrapper* File) = 0;		// returns false if an error was encountered
		virtual void					SaveForm(BGSEditorExtender::BGSEEPluginFileWrapper* File, BGSEditorExtender::BGSEEFormWrapper* Form);
	public:
		ICSEFormCollectionSerializer();
		virtual ~ICSEFormCollectionSerializer();

		enum
		{
			kSerializer_Unknown = 0,
			kSerializer_DefaultForm,
			kSerializer_ObjectRef,

			kSerializer__MAX
		};

		static UInt8					GetFileSerializerType(BGSEditorExtender::BGSEEPluginFileWrapper* File);				// returns the type of serializer used to create the file
		static void						SetFileSerializerType(BGSEditorExtender::BGSEEPluginFileWrapper* File, UInt8 Type);	// tags the file with the serializer type

		virtual UInt8					GetType() = 0;
	};

	class ICSEFormCollectionInstantiator
	{
	public:
		virtual ~ICSEFormCollectionInstantiator() = 0
		{
			;//
		}

		virtual bool					Instantiate(ICSEFormCollectionSerializer* Data, bool FreeTempData) = 0;		// returns false if an error occurred
	};

	// creates non-temp copies of the loaded forms and adds them to the datahandler
	class CSEDefaultFormCollectionInstantiator : public ICSEFormCollectionInstantiator
	{
	public:
		virtual ~CSEDefaultFormCollectionInstantiator();

		virtual bool					Instantiate(ICSEFormCollectionSerializer* Data, bool FreeTempData = true);
	};

	// for non-TESObjectREFR forms
	class CSEDefaultFormCollectionSerializer : public ICSEFormCollectionSerializer
	{
		friend class CSEDefaultFormCollectionInstantiator;
	protected:
		virtual bool					LoadForm(BGSEditorExtender::BGSEEPluginFileWrapper* File);		// returns false if an error was encountered
	public:
		virtual ~CSEDefaultFormCollectionSerializer();

		virtual bool					Serialize(BGSEditorExtender::BGSEEFormListT& Forms, BGSEditorExtender::BGSEEPluginFileWrapper* OutputStream);
		virtual bool					Deserialize(BGSEditorExtender::BGSEEPluginFileWrapper* InputStream, int& OutDeserializedFormCount);

		virtual UInt8					GetType();
	};

	class CSEObjectRefCollectionSerializer;

	class CSEObjectRefCollectionInstantiator : public ICSEFormCollectionInstantiator
	{
	protected:
		void							GetPositionOffset(TESObjectREFRListT& InData, NiNode* CameraNode, Vector3& OutOffset);
	public:
		virtual ~CSEObjectRefCollectionInstantiator();

										// creates references at the render window camera position
		virtual bool					Instantiate(ICSEFormCollectionSerializer* Data, bool FreeTempData = true);

										// generates 3D data for the loaded refs, returns false on error
		bool							CreatePreviewNode(CSEObjectRefCollectionSerializer* Data,
														  TESPreviewControl* PreviewControl,
														  TESObjectREFRListT& OutPreviewRefs,
														  NiNode** OutPreviewNode);
	};

	class CSEObjectRefCollectionSerializer : public ICSEFormCollectionSerializer
	{
		friend class CSEObjectRefCollectionInstantiator;
	protected:
		typedef std::map<TESObjectREFR*, TESObjectREFR*>		RefParentTableT;
		typedef std::map<TESObjectREFR*, bool>					RefParentStateTableT;

		TESFormListT					BaseFormDeserializatonBuffer;
		RefParentTableT					ParentDeserializationBuffer;				// maps (loaded) refs to their parents
		RefParentStateTableT			ParentStateDeserializationBuffer;			// maps (loaded) refs to their ESP opposite state

		virtual bool					LoadForm(BGSEditorExtender::BGSEEPluginFileWrapper* File);

		void							FreeDeserializationBuffers();
	public:
		CSEObjectRefCollectionSerializer();
		virtual ~CSEObjectRefCollectionSerializer();

		virtual bool					Serialize(BGSEditorExtender::BGSEEFormListT& Forms, BGSEditorExtender::BGSEEPluginFileWrapper* OutputStream);
		virtual bool					Deserialize(BGSEditorExtender::BGSEEPluginFileWrapper* InputStream, int& OutDeserializedFormCount);

		virtual UInt8					GetType();
	};
}