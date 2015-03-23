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

		TESFile*							GetWrappedPlugin(void) const;
	};

	class CSEFormListBuilder
	{
		BGSEditorExtender::BGSEEFormListT		FormList;
	public:
		CSEFormListBuilder();
		~CSEFormListBuilder();

		void									Add(TESForm* Form);
		bool									Copy(void);
	};
}