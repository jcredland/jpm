/* 
	This program is free software : you can redistribute it and / or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include <iostream>
#include "JucerFile.h"
#include "ModuleGenerator.h"
#include "Module.h"
#include "Directory.h"
#include "ConfigFile.h"

class App
{
public:
	App(StringArray commandLine_)
		:
		config(File::getCurrentWorkingDirectory().getChildFile("jpmfile.xml")),
		directory(URL("http://www.credland.net/jpm_directory.xml")),
		commandLine(commandLine_)
	{
		jucer.setFile(File::getCurrentWorkingDirectory());
	}

	~App()
	{
		jucer.save();
	}


	void run()
	{
		String command = commandLine[1];
		commandLine.removeRange(0, 2); 

		if (command == "list")
			list();
        else if (command == "install")
			install();
        else if (command == "genmodule")
			genmodule();
        else if (command == "rebuildjucer")
			rebuildJucerModuleList();
        else if (command == "erasecache")
			DownloadCache().clearCache();
        else if (command == "add")
            add();
        else
            printError("command not found"); 
	}

private:
	/** Add a modules to the jpmfile.xml and install it. */
	void addModuleFromDirectory(const String & moduleName)
	{
		auto modules = directory.getModulesByName(moduleName);

		if (modules.size() == 0)
		{
			std::cerr << "error: module " << moduleName << " not found" << std::endl;
			return;
		}

		if (modules.size() > 1)
            std::cout << "Installing multiple modules" << std::endl;

        for (auto module: modules)
        {
            std::cout << module.getSummaryString() << std::endl;
            
    		if (module.isValid())
    		{
                module.install(File::getCurrentWorkingDirectory().getChildFile("jpm_modules"));
                config.addModule(module);
    		}
        }
        
        rebuildJucerModuleList();
	}


    void addLocalModule(const String & userSuppliedPath)
    {
        File folder(File::getCurrentWorkingDirectory().getChildFile(userSuppliedPath)); 

        Module m;
        m.setSource("LocalPath"); 
        m.setName(folder.getFileNameWithoutExtension());
        m.setVersion("NA"); 
        m.setPath(userSuppliedPath); 
        config.addModule(m);

        rebuildJucerModuleList();
    }

    void addModuleToJucer(Module module)
    {
        if (module.getSource() == "LocalPath")
            jucer.addModule(module.getName(), module.getPath()); 
        else
            jucer.addModule(module.getName());
    }

	/** Update the jucer file with the latest list of modules. */
	void rebuildJucerModuleList()
	{
		jucer.clearModules();

		auto allModules = config.getModules();

		for (auto m : allModules)
			addModuleToJucer(m);
	}

	/** Installs any modules that are missing from the jpm_modules folder. */
	void installMissingModules()
	{
		auto allModules = config.getModules();

		for (auto module : allModules)
		{
			auto jpmModulesFolder = File::getCurrentWorkingDirectory().getChildFile("jpm_modules");
			auto name = module.getName(); 

			if (!jpmModulesFolder.getChildFile(name).exists())
			{
				std::cout << "installing: " << name << std::endl;
				module.install(jpmModulesFolder);
			}
		}
	}

    
	void install()
	{
		std::cout << "install..." << std::endl;
		if (commandLine.size() == 0)
		{
			std::cerr << "installing missing modules (to force a refresh delete the jpm_modules folder first)" << std::endl;
			installMissingModules();
		}
		else
		{
			while (commandLine.size() > 0)
			{
				addModuleFromDirectory(commandLine[0]);
				commandLine.remove(0);
			}
		}
	}
    
    void genmodule()
    {
        ModuleGenerator generator;
        generator.run(commandLine);
    }

	void list()
	{
		String searchString;

		if (commandLine.size() == 0)
			searchString = "*";
		else
			searchString = commandLine[0];

		auto result = directory.getModulesByName(searchString); 

		for (auto r: result)
			std::cout << r.getSummaryString() << std::endl;
	}

    void add()
    {
        if (commandLine.size() == 0)
            printError("You need to specify a folder with a module in it"); 
        else
            addLocalModule(commandLine[0]);
    }

	ConfigFile config;
	JucerFile jucer;	
	Directory directory;

	StringArray commandLine;
};


void usage()
{
    std::cout << std::endl;
    std::cout << "jpm - a juce package manager  (version 0.01)" << std::endl;
    std::cout << std::endl;
    std::cout << "ESSENTIAL COMMANDS" << std::endl;
    std::cout << "jpm install <source>      add and install a modules" << std::endl;
    std::cout << "jpm install               download any missing modules for the current project" << std::endl;
    std::cout << "jpm add <source>          add a local module without using the directory" << std::endl;
    std::cout << "jpm list [<wildcard>]     show all available modules, e.g. jpm list *core*" << std::endl;
    std::cout << "jpm erasecache            erase the download cache" << std::endl;
    std::cout << std::endl;
    std::cout << "OTHER COMMANDS" << std::endl;
    std::cout << "jpm genmodule <name>      create a module template [ beta ]" << std::endl;
    std::cout << "jpm rebuildjucer          rewrite the modules section of the jucer file" << std::endl;
    std::cout << std::endl;
    std::cout << "Run this from the root of your JUCE project" << std::endl;
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

	/* 
	This mega-try/catch probably isn't the best idea. Though it's a commandline failure and
	some hard-failure modes are probably not inappropriate. 
	*/
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
