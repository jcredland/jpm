
#ifndef REPOSITORY_H_INCLUDED
#define REPOSITORY_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Module.h"
#include "Utilities.h"
#include "DownloadCache.h"
#include <iostream>

/** Holds an index of where to find specific modules.  We may want more than one of these in the end. */
class Directory
{
public:
    /*
     * Directory entries are <repo>.  
     * <repo> has properties shortname, path and source. 
     * shortname - a non-unique identifier. 
     * source - determines the routines used for download and extraction (e.g. GitHub, LocalPath)
     * path - source dependant reference to the data
     *
     * <module> is a child of <repo> with properties:
     * name - name of module
     * description - description
     * subpath - again source dependent but used for finding the module
     */
	
    /**
     * Opens a directory with the given URL, downloading the latest version of
     * the contents. 
	*/
    Directory()
	{
		DownloadCache cache;
		String file = cache.downloadFromDatabase();

		if (!file.isEmpty())
        {
			directory = Database::parseToArray (file);
        }
		else
        {
            throw JpmFatalExcepton ("data empty, network or filesystem problem",
                                    "Check " 
                                    + cache.getCachedDatabaseLocation ().getFullPathName()
                                    + " for debugging which should contain any returned data");
		}
	}


	/** 
     * Find all the modules with a specific name.  moduleName can be specified
     * as to match a specific version in a specific repo: juce/juce_core@3.1.1
     * In this case it should match one entry, but there's no guarantee of
     * that!
	*/
	Array<Module> getModulesByName(const String & moduleNameString)
	{
		Array<Module> results;
		ModuleName module(moduleNameString);

		if (module.getRepo().isNotEmpty())
		{
			std::cout << "searching one repo" << std::endl;
            
            var repoEntry;
            if (directory.isArray())
            {
                for (auto repo : *directory.getArray())
                {
                    if (repo["shortname"] != var::null && repo["shortname"] == module.getRepo())
                    {
                        repoEntry = repo;
                    }
                }
            }

			if (repoEntry.isVoid())
			{
				std::cerr << "repo not found " << module.getRepo() << std::endl;
				return results;
			}
            
            DBG ("get matching modules from " << repoEntry["shortname"].toString() << "...");

			results.addArray(getMatchingModulesFromRepo(repoEntry, module.getName()));
		} 
		else
		{
            if (directory.isArray())
            {
                for (auto repoEntry : *directory.getArray())
                {
                    DBG ("get matching modules from " << repoEntry["shortname"].toString() << "...");
                    results.addArray(getMatchingModulesFromRepo(repoEntry, module.getName()));
                }
            }
            else
            {
                std::cerr << "error - directory is not an array" << std::endl;
            }
		}

		std::cout << "found: " << results.size() << std::endl;

		/* If a version was provided then set it.  We won't know until download time whether it definitely exists. */
		if (module.getVersion().isNotEmpty())
			for (auto & a : results)
				a.setVersion(module.getVersion());

		return results;
	}

private:
    
    bool matchModule (var moduleEntry, String name)
    {
        return moduleEntry["name"].toString().matchesWildcard(name, false);
    }
    
    Module createModuleFromEntry(var repoEntry, var moduleEntry)
    {
        auto test = [&repoEntry](const String & text)
        {
            if (text.isEmpty())
                std::cerr << "warning: error in directory for repo " << repoEntry["shortname"].toString() << std::endl;
            return text;
        };
        
        Module m;
        
        m.setRepo(test(repoEntry["shortname"]));
        m.setPath(test(repoEntry["path"]));
        m.setSource(test(repoEntry["source"]));
        m.setName(test(moduleEntry["name"]));
        m.setSubPath(test(moduleEntry["subpath"]));
        
        /* Description is allowed to be empty. */
        m.setDescription(moduleEntry["description"]);
        
        return m;
    }
    
	Array<Module> getMatchingModulesFromRepo(var repoEntry, const String & name)
	{
		Array<Module> result;

        if (repoEntry["module"].isArray())
        {
            for (auto moduleEntry : *repoEntry["module"].getArray())
            {
                if (matchModule (moduleEntry, name))
                    result.add (createModuleFromEntry (repoEntry, moduleEntry));
            }
        }
        else
        {
            var moduleEntry = repoEntry["module"];
            if (matchModule (moduleEntry, name))
                result.add (createModuleFromEntry (repoEntry, moduleEntry));
            
        }
		return result;
	}
    
	var directory;
};

#endif  // REPOSITORY_H_INCLUDED
