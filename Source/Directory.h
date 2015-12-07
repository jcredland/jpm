
#ifndef REPOSITORY_H_INCLUDED
#define REPOSITORY_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Module.h"
#include "DownloadCache.h"
#include <iostream>

/** Provides an STL compatible iterator for the children of ValueTree. */
class ValueTreeChildrenConnector
{
public:
	ValueTreeChildrenConnector(const ValueTree & tree) : tree(tree) {}

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

/** Decodes module names in the format repo/name@version */
class ModuleName
{
public:
	ModuleName(const String & name_) : name(name_) {}

	String getRepo() const { return name.contains("/") ? name.upToFirstOccurrenceOf("/", false, false) : String::empty; }

	String getVersion() const { return name.contains("@") ? name.fromLastOccurrenceOf("@", false, false) : String::empty; }

	String getName() const
	{
		if (name.contains("/"))
			return name.fromFirstOccurrenceOf("/", false, false).upToLastOccurrenceOf("@", false, false);

		return name.upToLastOccurrenceOf("@", false, false);
	}
private:
	String name;
};

/** Holds an index of where to find specific modules.  We may want more than one of these in the end. */
class Directory
{
public:
	/**
	Directory URL file has the following format: 
	
	<repo shortname="juce" path="https://github.com/julianstorer/JUCE/archive/" source="GitHub">
		<module name="juce_core" description="Some helpful information about the module">
			<version version="3.1.1" subpath="JUCE-3.1.1/modules/juce_core"/>
			<version version="4.1.1" subpath="JUCE-4.1.1/modules/juce_core"/>
		</module>
	</source>

	repo - used to differentiate different hosts for the same code

	"source" determines the engine used to retrieve the module.  GitHub for 
	example appends the version number to the URL then downloads the zip file and
	expands it.  Then looks in the subpath under that expanded zip file for the 
	module source. 
	*/
	Directory(URL location)
	{
		DownloadCache cache;
		String file = cache.downloadTextFile(location);
		ScopedPointer<XmlElement> xml = XmlDocument(file).getDocumentElement();

		if (xml)
			directory = ValueTree::fromXml(*xml);
		else
			std::cerr << "Xml format error with directory:" << file << std::endl;
	}


	/** 
	Find all the modules with a specific name. 
	moduleName can be specified as to match a specific version in a specific repo: juce/juce_core@3.1.1
	In this case it should match one entry, but there's no guarantee of that!
	*/
	Array<Module> getModulesByName(const String & moduleNameString)
	{
		Array<Module> results;

		ModuleName module(moduleNameString);

		if (module.getRepo().isNotEmpty())
		{
			std::cout << "searching one repo" << std::endl;
			auto repoEntry = directory.getChildWithProperty("shortname", module.getRepo());

			if (repoEntry == ValueTree::invalid)
			{
				std::cerr << "repo not found " << module.getRepo() << std::endl;
				return results;
			}

			results.addArray(getMatchingModulesFromRepo(repoEntry, module.getName()));
		} 
		else
		{
			for (auto repoEntry : ValueTreeChildrenConnector(directory))
				results.addArray(getMatchingModulesFromRepo(repoEntry, module.getName()));
		}

		std::cout << "found: " << results.size() << std::endl;

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
				result.add(m);
			}
		}
		return result;
	}
	ValueTree directory;
};



#endif  // REPOSITORY_H_INCLUDED
