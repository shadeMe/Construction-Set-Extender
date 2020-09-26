#pragma once

namespace bgsee
{
	// Always relative to BasePath, which is Data\BGSEE
	class ResourceLocation
	{
		std::string		RelativePath;

		bool			CheckPath(void);
		static void		AnnealPath(std::string& Path);
	public:
		ResourceLocation();
		ResourceLocation(std::string Path);
		~ResourceLocation();

		ResourceLocation&	operator=(const ResourceLocation& rhs);
		ResourceLocation&	operator=(std::string rhs);
		std::string			operator()() const;				// returns the full path

		std::string			GetFullPath() const;			// returns BasePath + RelativePath
		std::string			GetRelativePath() const;		// return RelativePath

		bool				IsFile() const;
		bool				IsDirectory() const;
		bool				Exists() const;
		ResourceLocation	GetCurrentDirectory() const;	// returns the parent directory it it's a file, *this otherwise
		ResourceLocation	GetParentDirectory() const;		// returns the parent directory of the path (up to the BasePath)
		std::string			GetExtension() const;			// returns an empty string for directories


		static const std::string&	GetBasePath(void);
		static bool					IsRelativeTo(const ResourceLocation& Path, const ResourceLocation& RelativeTo);
		static bool					IsRelativeTo(const std::string& Path, const ResourceLocation& RelativeTo);
		static std::string			ExtractRelative(const std::string& Path, const std::string& RelativeTo);
	};
}