
#ifndef REPOSITORY_H_INCLUDED
#define REPOSITORY_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Module.h"
#include "Utilities.h"
#include "DownloadCache.h"
#include <iostream>


	class Iterator
	{
	public:
		Iterator(ValueTree & tree, int position) : tree(tree), pos(position) {}
		Iterator & operator++() { ++pos; return *this; }
		bool operator!= (const Iterator & other) const { return other.pos != pos || other.tree != tree; }
		ValueTree operator * () const { return tree.getChild(pos); }

	private:
		Iterator & operator=(const Iterator &) = delete;
		ValueTree & tree;
		int pos;
	};

	Iterator begin() { return Iterator(tree, 0); }
	Iterator end() { return Iterator(tree, tree.getNumChildren()); }

private:
	JUCE_DECLARE_NON_COPYABLE(ValueTreeChildrenConnector)
		ValueTree tree;
};


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
	Directory(URL location)
	{
		DownloadCache cache;
		String file = cache.downloadTextFile(location);
		ScopedPointer<XmlElement> xml = XmlDocument(file).getDocumentElement();

		if (xml)
        {
			directory = ValueTree::fromXml(*xml);
        }
		else
        {
            throw JpmFatalExcepton ("directory format error or network problem",
                                    "Check " 
                                    + cache.getCachedFileLocation (location).getFullPathName() 
                                    + " for debugging which should contain the contents of " 
                                    + location.toString (true));
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

        for (auto repoEntry : ValueTreeChildrenConnector (directory))
		{
            if (module.getRepo().isEmpty() || repoEntry["shortname"] == module.getRepo())
			results.addArray(getMatchingModulesFromRepo(repoEntry, module.getName()));
		} 

		/* If a version was provided then set it.  We won't know until download time whether it definitely exists. */
		if (module.getVersion().isNotEmpty())
			for (auto & a : results)
				a.setVersion(module.getVersion());

		return results;
	}

private:
	Array<Module> getMatchingModulesFromRepo(ValueTree repoEntry, const String & name)
	{
		Array<Module> result;

		for (auto moduleEntry: ValueTreeChildrenConnector(repoEntry))
		{
			if (moduleEntry["name"].toString().matchesWildcard(name, false))
			{
                /* We use this short lambda for validating the mandatory fields
                 * in the directory. */
				auto test = [&repoEntry](const String & text)
				{
					if (text.isEmpty())
                        printWarning("warning: error in directory for repo " + repoEntry["shortname"].toString());

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
                
				result.add(m);
			}
		}

		return result;
	}
	ValueTree directory;
};

#endif  // REPOSITORY_H_INCLUDED
