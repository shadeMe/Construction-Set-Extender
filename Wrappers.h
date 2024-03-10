#pragma once

#include "[BGSEEBase]\Wrappers.h"

namespace cse
{
	class TESFormWrapper : public bgsee::FormWrapper
	{
	protected:
		TESForm*							WrappedForm;
	public:
		TESFormWrapper(TESForm* Form);
		virtual ~TESFormWrapper();

		virtual UInt32						GetFormID(void) const;
		virtual const char*					GetEditorID(void) const;
		virtual UInt8						GetType(void) const;
		virtual const char*					GetTypeString(void) const;
		virtual UInt32						GetFlags(void) const;

		virtual bool						GetIsDeleted(void) const;

		TESForm*							GetWrappedForm(void) const;
	};

	class TESFileWrapper : public bgsee::PluginFileWrapper
	{
	protected:
		TESFile*							WrappedPlugin;
		std::string							PluginPath;

		void								CreateTempFile(void);
	public:
		TESFileWrapper();
		virtual ~TESFileWrapper();

		virtual bool						Construct(const char* FileName, bool OverwriteExisting);
		virtual void						Purge(void);

		virtual bool						Open(bool ForWriting);
		virtual bool						SaveHeader(void);
		virtual bool						CorrectHeader(UInt32 RecordCount);
		virtual bool						Close(void);

		virtual UInt8						GetRecordType(void);
		virtual bool						GetNextRecord(bool SkipIgnoredRecords);

		virtual int							GetErrorState(void) const;
		virtual const char*					GetFileName(void) const;
		virtual void						Delete(void);

		TESFile*							GetWrappedPlugin(void) const;
	};

	class IFormCollectionSerializer : public bgsee::FormCollectionSerializer
	{
		static const char*				kSigilDefaultForm;
		static const char*				kSigilObjectRef;
	protected:
		TESFormArrayT					LoadedFormBuffer;		// stores loaded forms for deferred linking

		void							FreeBuffer(void);

		virtual bool					LoadForm(bgsee::PluginFileWrapper* File) = 0;		// returns false if an error was encountered
		virtual void					SaveForm(bgsee::PluginFileWrapper* File, bgsee::FormWrapper* Form);

		bool							GetFormInBuffer(TESForm* Form) const;
	public:
		IFormCollectionSerializer();
		virtual ~IFormCollectionSerializer();

		enum
		{
			kSerializer_Unknown = 0,
			kSerializer_DefaultForm,
			kSerializer_ObjectRef,

			kSerializer__MAX
		};

		static UInt8					GetFileSerializerType(bgsee::PluginFileWrapper* File);				// returns the type of serializer used to create the file
		static void						SetFileSerializerType(bgsee::PluginFileWrapper* File, UInt8 Type);	// tags the file with the serializer type

		virtual UInt8					GetType() = 0;
	};

	class IFormCollectionInstantiator
	{
	public:
		virtual ~IFormCollectionInstantiator() = 0
		{
			;//
		}

		virtual bool					Instantiate(IFormCollectionSerializer* Data, bool FreeTempData) = 0;		// returns false if an error occurred
	};

	// creates non-temp copies of the loaded forms and adds them to the datahandler
	class DefaultFormCollectionInstantiator : public IFormCollectionInstantiator
	{
	public:
		virtual ~DefaultFormCollectionInstantiator();

		virtual bool					Instantiate(IFormCollectionSerializer* Data, bool FreeTempData = true);
	};

	// for non-TESObjectREFR forms
	class DefaultFormCollectionSerializer : public IFormCollectionSerializer
	{
		friend class DefaultFormCollectionInstantiator;
	protected:
		virtual bool					LoadForm(bgsee::PluginFileWrapper* File);		// returns false if an error was encountered
	public:
		virtual ~DefaultFormCollectionSerializer();

		virtual bool					Serialize(bgsee::FormListT& Forms, bgsee::PluginFileWrapper* OutputStream);
		virtual bool					Deserialize(bgsee::PluginFileWrapper* InputStream, int& OutDeserializedFormCount);

		virtual UInt8					GetType();
	};

	class ObjectRefCollectionSerializer;

	class ObjectRefCollectionInstantiator : public IFormCollectionInstantiator
	{
	protected:
		void							GetPositionOffset(TESObjectREFRArrayT& InData, NiNode* CameraNode, Vector3& OutOffset);
		TESObject*						InstantiateBaseForm(TESObjectREFR* Ref);
	public:
		virtual ~ObjectRefCollectionInstantiator();

										// creates references at the render window camera position
		virtual bool					Instantiate(IFormCollectionSerializer* Data, bool FreeTempData = true);

										// generates 3D data for the loaded refs, returns false on error
										// caller takes ownership of the output
		bool							CreatePreviewNode(ObjectRefCollectionSerializer* Data,
														  TESPreviewControl* PreviewControl,
														  TESFormArrayT& OutPreviewBaseForms,
														  TESObjectREFRArrayT& OutPreviewRefs,
														  NiNode** OutPreviewNode);
	};

	// describes a object ref in a collection
	struct ObjectRefDescriptor
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

	typedef std::shared_ptr<ObjectRefDescriptor>				ObjectRefDescriptorHandleT;
	typedef std::vector<ObjectRefDescriptorHandleT>				ObjectRefCollectionDescriptorArrayT;

	// includes a shallow copy of the refs' baseforms (dependencies are ignored)
	class ObjectRefCollectionSerializer : public IFormCollectionSerializer
	{
		friend class ObjectRefCollectionInstantiator;
	protected:
		typedef std::map<TESObjectREFR*, TESObjectREFR*>		RefParentTableT;
		typedef std::map<TESObjectREFR*, bool>					RefParentStateTableT;

		TESFormArrayT					BaseFormDeserializatonBuffer;
		RefParentTableT					ParentDeserializationBuffer;				// maps (loaded) refs to their parents
		RefParentStateTableT			ParentStateDeserializationBuffer;			// maps (loaded) refs to their ESP opposite state
		bool							StrictBaseFormResolution;					// if true, loaded refs' baseforms must resolve to an existing form; if not, they are discarded
																					// if false, loaded refs are allowed dependencies on the deserialized baseforms; new baseform instances must be created and linked during the instantiation

		virtual bool					LoadForm(bgsee::PluginFileWrapper* File);

		void							FreeDeserializationBuffers();
		bool							IsBaseFormTemporary(TESForm* Form) const;	// returns true if the form is found in the deserialization buffer
		bool							IsBaseFormTemporary(UInt32 FormID) const;
		bool							ResolveBaseForms();							// returns false if the resolution wasn't successful

		ObjectRefDescriptor*			GetDescriptor(TESForm* Form) const;
	public:
		ObjectRefCollectionSerializer(bool StrictBaseFormResolution);
		virtual ~ObjectRefCollectionSerializer();

		virtual bool					Serialize(bgsee::FormListT& Forms, bgsee::PluginFileWrapper* OutputStream);
		virtual bool					Deserialize(bgsee::PluginFileWrapper* InputStream, int& OutDeserializedFormCount);

		virtual UInt8					GetType();

		bool							GetHasTemporaryBaseForms() const;			// returns true if any of the loaded refs are dependent on a temp baseform
		void							GetDescription(ObjectRefCollectionDescriptorArrayT& Out) const;
	};
}