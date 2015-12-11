
#ifndef DOWNLOADCACHE_H_INCLUDED
#define DOWNLOADCACHE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Database.h"
#include <iostream>

/**
 * Stores downloaded files in a temporary loction and reuses those temporary
 * files when a download is requested.
*/
class DownloadCache
{
public:
    const int cacheRefreshMinutes { 60 };

    DownloadCache()
    {
        location = File::getSpecialLocation (File::userApplicationDataDirectory).getChildFile ("jpm.modulecache");
        location.createDirectory();
    }

    void clearCache()
    {
        location.deleteRecursively();
        location.createDirectory();
    }

    /**
     * Downloads a text file from the internet, stores it in the cache,
     * if the site isn't available or the downloaded file is empty it
     * uses the cached file if available.
     */
    String downloadTextFile (URL remoteFile)
    {
        auto cachedFile = getCachedFileLocation (remoteFile);

        if (cachedFile.exists() && isRecent (cachedFile))
            return cachedFile.loadFileAsString();

        String result = remoteFile.readEntireTextStream();

        if (result.isEmpty())
            return cachedFile.loadFileAsString(); /* fallback to cached version. */

        cachedFile.replaceWithText (result);
        return result;
    }

    /**
     * Makes a database query to get the directory.  Caches it in the
     * download cache.
     */
    String downloadFromDatabase()
    {
        auto target = getCachedDatabaseLocation();

        if (target.exists() && isRecent (target))
            return target.loadFileAsString();

        String result = database.getAllModulesAsJSON();

        target.replaceWithText (result);

        return result;
    }

    File getCachedFileLocation (const URL& urlToGet) const
    {
        int64 hash = urlToGet.toString (true).hashCode64();
        return location.getChildFile (String (hash));
    }

    File getCachedDatabaseLocation() const
    {
        return getCachedFileLocation (database.getURL());
    }

    /** Return true if the file is less than an hour old. */
    bool isRecent (const File& filename)
    {
        auto modTime = filename.getLastModificationTime();
        auto curTime = Time::getCurrentTime();
        auto age = RelativeTime::milliseconds (curTime.toMilliseconds() - modTime.toMilliseconds());
        return age < RelativeTime::minutes (cacheRefreshMinutes);
    }

    /**
     * Downloads a file or retrieves it from the cache.  Returns a location
     * where the file can be found.  Displays error messages for all failure
     * modes. */
    File downloadUrlAndUncompress (URL urlToGet)
    {
        auto target = getCachedFileLocation (urlToGet);

        if (target.exists() && isRecent (target))
            return target;

        MemoryBlock memoryBlock;

        if (! readEntireBinaryStreamWithProgressBar (urlToGet, memoryBlock))
        {
            auto urlString = urlToGet.toString (false);

            if (target.exists())
            {
                printWarning ("error downloading file - using cached version of " + urlString);
                return target;
            }
            else
            {
                printError ("No suitable file cached - aborting - could not download " + urlString);
                return File::nonexistent;
            }
        }

        MemoryInputStream inputStream (memoryBlock, false);

        printInfo ("uncompressing to " + target.getFullPathName());

        ZipFile zip (inputStream);
        auto result = zip.uncompressTo (target, true);

        if (result.failed())
        {
            printError (result.getErrorMessage());
            return File::nonexistent;
        }

        return target;
    }

private:
    static bool progressBar (void* context, int bytesSent, int totalBytes)
    {
        std::cout << "Connecting..." << bytesSent << "/" << totalBytes << "\r";
        return true; /* means continue the download. */
    }

    static bool readEntireBinaryStreamWithProgressBar (URL url, MemoryBlock& destData)
    {
        const ScopedPointer<InputStream> in (url.createInputStream (false,
                                             &DownloadCache::progressBar));

        if (in != nullptr)
        {
            int64 total = 0;
            int64 numBytesReceived = 0;
            int64 totalAvailable = in->getTotalLength();

            do
            {
                MemoryOutputStream mo (destData, true);
                numBytesReceived = mo.writeFromInputStream (*in, 4096);
                total += numBytesReceived;
                std::cout
                        << "download progress ... "
                        << total / 1024
                        << "k of "
                        << totalAvailable / 1024
                        << "k             \r";
            }
            while (numBytesReceived == 4096);

            std::cout << std::endl;

            return true;
        }

        return false;
    }

    File location;
    Database database;
};



#endif  // DOWNLOADCACHE_H_INCLUDED
