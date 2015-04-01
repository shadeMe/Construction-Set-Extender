#pragma once

namespace BGSEditorExtender
{
	// wraps around the base form class in BGS games (TESForm mostly)
	class BGSEEFormWrapper
	{
	public:
		virtual ~BGSEEFormWrapper() = 0
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

	typedef std::vector<BGSEEFormWrapper*>	BGSEEFormListT;

	// wraps around the BGS plugin file class (TESFile)
	class BGSEEPluginFileWrapper
	{
	public:
		virtual ~BGSEEPluginFileWrapper() = 0
		{
			;//
		}

		virtual bool						Construct(const char* FileName) = 0;			// creates and initializes an instance of the wrapped class
		virtual void						Purge(void) = 0;								// deletes the file

		virtual bool						Open(bool ForWriting) = 0;						// these return false on error
		virtual bool						SaveHeader(void) = 0;
		virtual bool						CorrectHeader(UInt32 RecordCount) = 0;
		virtual bool						Close(void) = 0;

		virtual UInt8						GetRecordType(void) = 0;
		virtual bool						GetNextRecord(bool SkipIgnoredRecords) = 0;

		virtual int							GetErrorState(void) const = 0;
		virtual const char*					GetFileName(void) const = 0;
	};

	// serialize/deserialize forms to/from a plugin file
	class BGSEEFormCollectionSerializer
	{
	public:
		virtual ~BGSEEFormCollectionSerializer() = 0
		{
			;//
		}

		virtual bool						Serialize(BGSEEFormListT& Forms, BGSEEPluginFileWrapper* OutputStream) = 0;		// doesn't clear the formlist, returns true if successful
		virtual bool						Deserialize(BGSEEPluginFileWrapper* InputStream, int& OutDeserializedFormCount) = 0;	// returns true if successful
	};
}