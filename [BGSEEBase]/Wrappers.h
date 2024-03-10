#pragma once

namespace bgsee
{
	// wraps around the base form class in BGS games (TESForm mostly)
	class FormWrapper
	{
	public:
		virtual ~FormWrapper() = 0
		{
			;//
		}

		virtual UInt32						GetFormID(void) const = 0;
		virtual const char*					GetEditorID(void) const = 0;
		virtual UInt8						GetType(void) const = 0;
		virtual const char*					GetTypeString(void) const = 0;
		virtual UInt32						GetFlags(void) const = 0;

		virtual bool						GetIsDeleted(void) const = 0;
	};

	typedef std::vector<FormWrapper*>	FormListT;

	// wraps around the BGS plugin file class (TESFile)
	class PluginFileWrapper
	{
	public:
		virtual ~PluginFileWrapper() = 0
		{
			;//
		}

		virtual bool						Construct(const char* FileName, bool OverwriteExisting) = 0;			// creates and initializes an instance of the wrapped class, overwrite deletes the existing file and creates a zero-byte replacement in its place
		virtual void						Purge(void) = 0;								// deletes the contents of the file

		virtual bool						Open(bool ForWriting) = 0;						// these return false on error
		virtual bool						SaveHeader(void) = 0;
		virtual bool						CorrectHeader(UInt32 RecordCount) = 0;
		virtual bool						Close(void) = 0;

		virtual UInt8						GetRecordType(void) = 0;
		virtual bool						GetNextRecord(bool SkipIgnoredRecords) = 0;

		virtual int							GetErrorState(void) const = 0;
		virtual const char*					GetFileName(void) const = 0;
		virtual void						Delete(void) = 0;								// removes the file from disk
	};

	// serialize/deserialize forms to/from a plugin file
	class FormCollectionSerializer
	{
	public:
		virtual ~FormCollectionSerializer() = 0
		{
			;//
		}

		virtual bool						Serialize(FormListT& Forms, PluginFileWrapper* OutputStream) = 0;		// doesn't clear the formlist, returns true if successful
		virtual bool						Deserialize(PluginFileWrapper* InputStream, int& OutDeserializedFormCount) = 0;	// returns true if successful
	};
}