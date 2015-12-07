
#ifndef DOWNLOADCACHE_H_INCLUDED
#define DOWNLOADCACHE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include <iostream>

/** 
Stores downloaded files in a temporary loction and reuses those temporary files when 
a download is requested. 
*/
class DownloadCache
{
public:
	DownloadCache()
	{
		location = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("jpm.modulecache");
		location.createDirectory();
	}

	void clearCache()
	{
		location.deleteRecursively();
		location.createDirectory();
	}

	String downloadTextFile(URL file)
	{
		auto target = getCachedFileLocation(file); 

		if (target.exists() && isRecent(target))
			return target.loadFileAsString();

		String result = file.readEntireTextStream();

		target.replaceWithText(result); 

		return result;
	}

	File getCachedFileLocation(const URL& urlToGet)
	{
		int64 hash = urlToGet.toString(true).hashCode64();
		return location.getChildFile(String(hash));
	}

	/** Return true if the file is less than an hour old. */
	bool isRecent(const File & filename)
	{
		auto modTime = filename.getLastModificationTime();
		auto curTime = Time::getCurrentTime();
		auto age = RelativeTime::milliseconds(curTime.toMilliseconds() - modTime.toMilliseconds());
		return age < RelativeTime::hours(1);
	}

	/** Downloads a file or retrieves it from the cache.  Returns a location where the file can be found. */
	File downloadUrlAndUncompress(URL urlToGet)
	{
		auto target = getCachedFileLocation(urlToGet);

		if (target.exists() && isRecent(target))
			return target;
		
		MemoryBlock memoryBlock;

		if (!urlToGet.readEntireBinaryStream(memoryBlock, false))
		{
			std::cout << "error: downloading " + urlToGet.toString(false) << std::endl;
			if (target.exists())
				return target; 
			else
				return File::nonexistent;
		}

		MemoryInputStream inputStream(memoryBlock, false);

		std::cout << "Uncompressing to " << target.getFullPathName() << std::endl;
		ZipFile zip(inputStream);
		auto result = zip.uncompressTo(target, true); 

		if (result.failed())
		{
			std::cout << result.getErrorMessage() << std::endl;
			return File::nonexistent;
		}

		return target;
	}

	File location;
};



#endif  // DOWNLOADCACHE_H_INCLUDED
