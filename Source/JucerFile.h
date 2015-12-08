/*
  ==============================================================================

    JucerFile.h
    Created: 7 Dec 2015 11:52:37am
    Author:  Jim

  ==============================================================================
*/

#ifndef JUCERFILE_H_INCLUDED
#define JUCERFILE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include <iostream>

class InvalidJucerFormat 
{};

class JucerFile
{
public:
	JucerFile()
		:
		jucer(ValueTree::invalid)
	{}

    bool setFile(const File & fileToLoad) 
    {

		if (fileToLoad.isDirectory())
		{
			Array<File> files; 
			fileToLoad.findChildFiles(files, File::findFiles, false, "*jucer"); 

			if (files.size() == 0)
			{
				std::cerr << "No jucer file found in " << fileToLoad.getFullPathName() << std::endl;
				return false;
			}

			if (files.size() > 1)
			{
				std::cerr << "Multiple jucer files found, please specify which one to update" << std::endl;
				return false;
			}

			file = files[0];
		}
		else
		{
			file = fileToLoad;
		}
		

		ScopedPointer<XmlElement> xml = XmlDocument(file).getDocumentElement();

		if (!xml)
		{
			std::cerr << "error: cannot parse XML" << std::endl;
			return false;
		}

		jucer = ValueTree::fromXml(*xml);
		return true;
    }

	void save()
	{
		file.replaceWithText(jucer.toXmlString());
	}

	ValueTree getAllModules()
	{
		return jucer.getChildWithName("MODULES"); 
	}

	void listModules()
	{
		auto modules = getAllModules();

		for (int i = 0; i < modules.getNumChildren(); ++i)
			std::cout << modules.getChild(i)["id"].toString() << std::endl;
	}

	Array<ValueTree> getAllExporterModules()
	{
		Array<ValueTree> results;

		auto exportFormats = jucer.getChildWithName("EXPORTFORMATS"); 

		if (exportFormats == ValueTree::invalid)
			throw InvalidJucerFormat();

		for (int i = 0; i < exportFormats.getNumChildren(); ++i)
		{
			auto modules = exportFormats.getChild(i).getChildWithName("MODULEPATHS"); 
			results.add(modules); 
		}

		return results;
	}

	/** 
	Here we assume we are going to have copies under jpm_modules and that therefore all the
	module paths will be the same for all plaforms. EXCEPT - where we are using a LocalPath
    source. */
	void addModule(const String & moduleName, const String & modulePath = "./jpm_modules")
    {
		{
			ValueTree modulePathTree("MODULEPATH");
			modulePathTree.setProperty("id", moduleName, nullptr);
			modulePathTree.setProperty("path", modulePath, nullptr);

			auto exporterModules = getAllExporterModules();
			for (auto k: exporterModules)
			{
				ValueTree t = modulePathTree.createCopy();
				k.addChild(t, -1, nullptr); 
			}
		}

		auto modules = getAllModules(); 
		ValueTree module("MODULE"); 
		module.setProperty("id", moduleName, nullptr); 
		module.setProperty("showAllCode", true, nullptr); 
		module.setProperty("useLocalCopy", false, nullptr);  /* We don't have to do this because we have a local copy! */
		modules.addChild(module, -1, nullptr); 
    }

	String toString() const { return jucer.toXmlString(); }

	void clearModules()
	{
		auto exporterModules = getAllExporterModules(); 

		for (auto v : exporterModules)
			v.removeAllChildren(nullptr); 

		auto modules = getAllModules(); 

		modules.removeAllChildren(nullptr); 
	}
private:
	File file;
	ValueTree jucer;
};



#endif  // JUCERFILE_H_INCLUDED
