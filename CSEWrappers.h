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
		void								Delete(void);
	};

	class ICSEFormCollectionSerializer : public BGSEditorExtender::BGSEEFormCollectionSerializer
	{
	protected:
		TESFormListT					LoadedFormBuffer;		// stores loaded forms for deferred linking

		void							FreeBuffer(void);

		virtual bool					LoadForm(BGSEditorExtender::BGSEEPluginFileWrapper* File) = 0;		// returns false if an error was encountered
		virtual void					SaveForm(BGSEditorExtender::BGSEEPluginFileWrapper* File, BGSEditorExtender::BGSEEFormWrapper* Form);

		bool							GetFormInBuffer(TESForm* Form) const;
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
		TESObject*						InstantiateBaseForm(TESObjectREFR* Ref);
	public:
		virtual ~CSEObjectRefCollectionInstantiator();

										// creates references at the render window camera position
		virtual bool					Instantiate(ICSEFormCollectionSerializer* Data, bool FreeTempData = true);

										// generates 3D data for the loaded refs, returns false on error
										// caller takes ownership of the output
		bool							CreatePreviewNode(CSEObjectRefCollectionSerializer* Data,
														  TESPreviewControl* PreviewControl,
														  TESFormListT& OutPreviewBaseForms,
														  TESObjectREFRListT& OutPreviewRefs,
														  NiNode** OutPreviewNode);
	};

	// describes a object ref in a collection
	struct CSEObjectRefDescriptor
	{
		std::string			FormID;
		std::string			EditorID;
		std::string			Position;
		std::string			Rotation;
		std::string			Scale;

		std::string			BaseFormEditorID;
		std::string			BaseFormType;
		bool				TemporaryBaseForm;

		bool				HasEnableStateParent;
		std::string			EnableStateParentFormID;
		std::string			EnableStateParentEditorID;
		bool				ParentOppositeState;
	};

	typedef boost::shared_ptr<CSEObjectRefDescriptor>				ObjectRefDescriptorHandleT;
	typedef std::vector<ObjectRefDescriptorHandleT>					ObjectRefCollectionDescriptorListT;

	// includes a shallow copy of the refs' baseforms (dependencies are ignored)
	class CSEObjectRefCollectionSerializer : public ICSEFormCollectionSerializer
	{
		friend class CSEObjectRefCollectionInstantiator;
	protected:
		typedef std::map<TESObjectREFR*, TESObjectREFR*>		RefParentTableT;
		typedef std::map<TESObjectREFR*, bool>					RefParentStateTableT;

		TESFormListT					BaseFormDeserializatonBuffer;
		RefParentTableT					ParentDeserializationBuffer;				// maps (loaded) refs to their parents
		RefParentStateTableT			ParentStateDeserializationBuffer;			// maps (loaded) refs to their ESP opposite state
		bool							StrictBaseFormResolution;					// if true, loaded refs' baseforms must resolve to an existing form; if not, they are discarded
																					// if false, loaded refs are allowed dependencies on the deserialized baseforms; new baseform instances must be created and linked during the instantiation

		virtual bool					LoadForm(BGSEditorExtender::BGSEEPluginFileWrapper* File);

		void							FreeDeserializationBuffers();
		bool							IsBaseFormTemporary(TESForm* Form) const;	// returns true if the form is found in the deserialization buffer
		bool							ResolveBaseForms();							// returns false if the resolution wasn't successful

		CSEObjectRefDescriptor*			GetDescriptor(TESForm* Form) const;
	public:
		CSEObjectRefCollectionSerializer(bool StrictBaseFormResolution);
		virtual ~CSEObjectRefCollectionSerializer();

		virtual bool					Serialize(BGSEditorExtender::BGSEEFormListT& Forms, BGSEditorExtender::BGSEEPluginFileWrapper* OutputStream);
		virtual bool					Deserialize(BGSEditorExtender::BGSEEPluginFileWrapper* InputStream, int& OutDeserializedFormCount);

		virtual UInt8					GetType();

		bool							GetHasTemporaryBaseForms() const;			// returns true if any of the loaded refs are dependent on a temp baseform
		void							GetDescription(ObjectRefCollectionDescriptorListT& Out) const;
	};
}