

#include "../JuceLibraryCode/JuceHeader.h"
#include <iostream>
#include "JucerFile.h"
#include "Module.h"
#include "Directory.h"


/** Holds the saved JPM project configuration.  Contains module sources, version numbers and so on. */
class ConfigFile
{
public:
	ConfigFile(const File & file_) 
		:
		file (file_),
		config("jpm_config")
	{
		ScopedPointer<XmlElement> xml = XmlDocument(file).getDocumentElement();

		if (!xml)
			return; /* Not unexpected - there might be no initial configuration. */

		config = ValueTree::fromXml(*xml);
	}

	~ConfigFile()
	{
		/* This operation always feels risky ... should we do some backup and validation? */
		file.replaceWithText(config.toXmlString());
	}

	/** Returns the array of Module objects from the configuration. */
	Array<Module> getModules()
	{
		Array<Module> result;

		for (int i = 0; i < config.getNumChildren(); ++i)
		{
			auto data = config.getChild(i); 
			result.add(Module(data)); 
		}

		return result; 
	}

	/** Adds a new module object to the configuration. */
	void addModule(Module m)
	{
		auto child = m.getStateAsValueTree();
		config.addChild(child, -1, nullptr); 
	}

	/** Returns the config as a string - this is for debugging. */
	String toString() const { return config.toXmlString(); }

private:
	File file;
	ValueTree config;
};


class App
{
public:
	App(StringArray commandLine_)
		:
		config(File::getCurrentWorkingDirectory().getChildFile("jpmfile.xml")),
		commandLine(commandLine_)
	{
		jucer.setFile(File::getCurrentWorkingDirectory());
		jucer.clearModules();
	}

	void addAndInstallModule(const String & moduleName)
	{
		auto module = directory.getModuleByName(moduleName);
		module.install(File::getCurrentWorkingDirectory().getChildFile("jpm_modules"));
		config.addModule(module);
		rebuildJucerModuleList();
	}

	void rebuildJucerModuleList()
	{
		auto allModules = config.getModules();

		for (auto m : allModules)
			jucer.addModule(m.getName());

		std::cout << "JUCER FILE CONTENTS" << std::endl;
		std::cout << jucer.toString() << std::endl;
	}

	void refreshInstalledModules()
	{
		
	}

	void run()
	{
		if (commandLine[1] == "install")
		{
			if (commandLine.size() < 3)
			{
				refreshInstalledModules();
			}
			else 
			{
				addAndInstallModule(commandLine[2]);
			}
		}
	}

private:
	Directory directory;
	ConfigFile config;
	JucerFile jucer;	

	StringArray commandLine;
};


void usage()
{
    std::cout << "jpm - a juce package manager" << std::endl << std::endl;
    std::cout << "jpm install <source>      add and install a package" << std::endl;
    std::cout << "jpm install               download any missing packages" << std::endl;
}


int main (int argc, char* argv[])
{
	StringArray commandLineArguments;

	for (int i = 0; i < argc; ++i)
		commandLineArguments.add(argv[i]); 

	if (commandLineArguments.size() <= 1)
	{
		usage();
		return 1;
	}

	try {
		App app(commandLineArguments);

		app.run(); 

	} 
	catch (InvalidJucerFormat)
	{
		std::cerr << "exception: invalid jucer file format" << std::endl;
		return 1;
	}

    return 0;
}
