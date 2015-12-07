/*
  ==============================================================================

    Source_GitHub.h
    Created: 7 Dec 2015 6:58:50pm
    Author:  Jim

  ==============================================================================
*/

#ifndef SOURCE_GITHUB_H_INCLUDED
#define SOURCE_GITHUB_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include <iostream>

class Source 
{
public:
	virtual ~Source() {}
	virtual StringArray getAvailableVersions() = 0; 
	struct DownloadInfo
	{
		String actualVersionNumber;
		File file;
		bool success {false};
	};
};

inline String trimSlashes(String text)
{
	if (text.endsWithChar('/')) 
		text = text.dropLastCharacters(1);

	if (text.startsWithChar('/')) 
		return text.substring(1);

	return text; 
}

class GitHubSource
	:
	public Source
{
public:
	DownloadInfo download(const String & path, const String & version, const String & subpath)
	{
		DownloadInfo downloadInfo;

		std::cout << "downloading: " << path << std::endl;

		URL url("https://www.github.com/" + trimSlashes(path) + "/archive/" + version + ".zip");
		std::cout << "url: " << url.toString(true) << std::endl;

		DownloadCache cache;
		auto file = cache.downloadUrlAndUncompress(url);

		if (file == File::nonexistent)
		{
			std::cerr << "Could not obtain file" << std::endl;
			return downloadInfo;
		}

		Array<File> subFolders; 
		file.findChildFiles(subFolders, File::findDirectories, false, "*"); 

		if (subFolders.size() != 1)
			std::cerr << "warning: download cache contains mutiple subfolders.  either github have change their api or you should clear your cache" << std::endl;

		downloadInfo.file = file.getChildFile(subFolders[0].getFileName()).getChildFile(subpath); 

		if (downloadInfo.file.exists())
			downloadInfo.success = true;
		else
			std::cerr << "error: could not find subpath" << std::endl;

		if (version == "master")
			downloadInfo.actualVersionNumber = getMasterGitCommitReference(path);
		else
			downloadInfo.actualVersionNumber = version;

		return downloadInfo;
	}

	String getMasterGitCommitReference(const String & path)
	{
		URL url("https://api.github.com/repos/" + trimSlashes(path) + "/commits/master"); 

		auto data = url.readEntireTextStream(false);
		std::cout << url.toString(true) << std::endl;
		auto json = JSON::fromString(data); 
		auto sha1 = json.getProperty("sha", String::empty).toString(); 

		std::cout << "got master commit at " << sha1 << std::endl;

		return sha1;
	}

	StringArray getAvailableVersions() override
	{
		return StringArray("master"); 
	}
};



#endif  // SOURCE_GITHUB_H_INCLUDED
