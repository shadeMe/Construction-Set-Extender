#include "ResourceLocation.h"

namespace bgsee
{
	ResourceLocation::ResourceLocation( std::string Path ) :
		RelativePath(Path)
	{
		SME_ASSERT(CheckPath() == true);
		AnnealPath(RelativePath);
	}

	ResourceLocation::ResourceLocation() :
		RelativePath("")
	{
		;//
	}

	ResourceLocation::~ResourceLocation()
	{
		;//
	}

	std::string ResourceLocation::GetFullPath() const
	{
		return GetBasePath() + RelativePath;
	}

	std::string ResourceLocation::GetRelativePath() const
	{
		return RelativePath;
	}

	bool ResourceLocation::IsFile() const
	{
		return IsDirectory() == false;
	}

	bool ResourceLocation::IsDirectory() const
	{
		if (Exists() == false)
			return false;
		else
			return GetFileAttributes(GetFullPath().c_str()) == FILE_ATTRIBUTE_DIRECTORY;
	}

	bool ResourceLocation::Exists() const
	{
		return GetFileAttributes(GetFullPath().c_str()) != INVALID_FILE_ATTRIBUTES;
	}

	ResourceLocation ResourceLocation::GetCurrentDirectory() const
	{
		if (IsDirectory())
			return *this;
		else
			return GetParentDirectory();
	}

	ResourceLocation ResourceLocation::GetParentDirectory() const
	{
		int Slash = RelativePath.rfind("\\");
		if (Slash == -1)
			return *this;
		else
			return RelativePath.substr(0, Slash);
	}

	ResourceLocation& ResourceLocation::operator=(const ResourceLocation& rhs)
	{
		this->RelativePath = rhs.RelativePath;

		return *this;
	}

	ResourceLocation& ResourceLocation::operator=( std::string rhs )
	{
		this->RelativePath = rhs;
		SME_ASSERT(CheckPath() == true);
		AnnealPath(RelativePath);

		return *this;
	}

	std::string ResourceLocation::operator()() const
	{
		return GetFullPath();
	}

	bool ResourceLocation::CheckPath( void )
	{
		std::string PathB(RelativePath), BaseB(GetBasePath());

		SME::StringHelpers::MakeLower(PathB);
		SME::StringHelpers::MakeLower(BaseB);

		return PathB.find(BaseB) == std::string::npos;
	}

	void ResourceLocation::AnnealPath(std::string& Path)
	{
		SME::StringHelpers::MakeLower(Path);

		for (size_t Index(Path.find("\\\\")); Index != std::string::npos; Index = Path.find("\\\\"))
			Path.erase(Index, 1);
	}

	std::string ResourceLocation::GetExtension() const
	{
		std::string Out;
		if (IsFile())
		{
			int Dot = RelativePath.rfind(".");
			int Slash = RelativePath.rfind("\\");

			if (Dot > Slash)
				Out = RelativePath.substr(Dot + 1);
		}

		return Out;
	}

	const std::string& ResourceLocation::GetBasePath(void)
	{
		// initialized here to ensure statically allocated BGSEEResourceLocation instances never trigger assertions inside CRTMain
		static const std::string kBasePath = "Data\\BGSEE\\";
		return kBasePath;
	}

	bool ResourceLocation::IsRelativeTo(const ResourceLocation& Path, const ResourceLocation& RelativeTo)
	{
		if (Path.GetRelativePath().find(RelativeTo.GetRelativePath()) == 0)
			return true;
		else
			return false;
	}

	bool ResourceLocation::IsRelativeTo(const std::string& Path, const ResourceLocation& RelativeTo)
	{
		std::string Annealed(Path);
		AnnealPath(Annealed);
		if (Annealed.find(RelativeTo.GetRelativePath()) == 0)
			return true;
		else
			return false;
	}

	std::string ResourceLocation::ExtractRelative(const std::string& Path, const std::string& RelativeTo)
	{
		std::string A(Path), B(RelativeTo);
		AnnealPath(A); AnnealPath(B);
		int Index = A.find(B);
		if (Index == 0)
			return A.substr(B.length() - 1);
		else
			return A;

	}

}

