
#include "../JuceLibraryCode/JuceHeader.h"
#include <iostream>
#include "JucerFile.h"
#include "ModuleGenerator.h"
#include "Module.h"
#include "Directory.h"
#include "Database.h"
#include "ConfigFile.h"
#include "UserConfig.h"
#include "Constants.h"

/**
 * Main application.  This will be used both interactively by the user and also
 * by the unit tests, so ensure you don't add anything that'll break testing. 
 *
 * At some point the commands will need refactoring out of this into some tidy
 * little units.
 */
class App
{
public:
    App (StringArray commandLine_)
        :
        config (File::getCurrentWorkingDirectory().getChildFile ("jpmfile.xml")),
        commandLine (commandLine_)
    {
    }

    ~App()
    {
        if (jucer)
            jucer->save();
    }

    /**
     * Main entry point.
     */
    void run()
    {
        printInfo ("juce package manager");
        String command = commandLine[1];
        commandLine.removeRange (0, 2);

        if (command == "list")
            list();
        else if (command == "install")
            install();
        else if (command == "genmodule")
            genmodule();
        else if (command == "adduser")
            adduser();
        else if (command == "publish")
            publish();
        else if (command == "rebuildjucer")
            rebuildjucer();
        else if (command == "erasecache")
            erasecache();
        else if (command == "add")
            add();
        else if (command == "zip") // test for folder compression method
        {
            Database db;
            ZipFileUtilities::compressFolder(File::getCurrentWorkingDirectory().getChildFile (commandLine[0]),
                                    File::getCurrentWorkingDirectory().getChildFile (commandLine[1]));
        }
        else if (command == "zipentries")
        {
            ZipFile zipFile (File::getCurrentWorkingDirectory().getChildFile (commandLine[0]));
            DBG ("Unzip: " << zipFile.getNumEntries() << " entries");
            
        }
        else
            printError ("command not found");
    }

private:
    /** Creates the JucerFile object and opens the jucer file 
     * if it's not already been done. */
    void openJucerFile()
    {
        if (! jucer)
        {
            jucer = new JucerFile();
            jucer->setFile (File::getCurrentWorkingDirectory());
        }
    }

    /** Add a modules to the jpmfile.xml and install it. */
    void addModuleFromDirectory (const String& moduleName)
    {
        Directory directory;
        auto modules = directory.getModulesByName (moduleName);

        if (modules.size() == 0)
        {
            std::cerr << "error: module " << moduleName << " not found" << std::endl;
            return;
        }

        if (modules.size() > 1)
            printInfo ("installing " + String (modules.size()) + " modules");

        for (auto module : modules)
        {
            printHeading ("installing: " + module.getRepo() + "/" + module.getName() + "@" + module.getVersion());

            if (module.isValid())
            {
                module.install (File::getCurrentWorkingDirectory().getChildFile ("jpm_modules"));
                config.addModule (module);
            }
        }

        rebuildjucer();
    }
    
    

    /** Add a modules to the jpmfile.xml and install it. */
    void installModule (const String& moduleId)
    {
        Database db;
        
        var data = db.getModuleById (moduleId);

        printHeading ("installing: " + data["name"].toString() + "@" + data["version"].toString());
        
        // install module: base64 decode _attachments and unzip to folder..
        
        Identifier moduleZipId (moduleId + ".zip");
        MemoryBlock zipData = db.getZippedSource (moduleId);
        //DEBUG
//        String debugStr;
//        char* dataPtr = (char*) zipData.getData();
//        for (int i=0; i<zipData.getSize(); i++)
//        {
//            debugStr += dataPtr[i];
//        }
//        DBG (debugStr);
        /////
        DBG ("zipped size: " + String (zipData.getSize()));
        MemoryInputStream is(zipData, false);
        
        // DEBUG
//        File outputZip (File::getCurrentWorkingDirectory().getChildFile (moduleId + ".zip"));
//        outputZip.replaceWithData(is.getData(), is.getDataSize());
//        ZipFile outputZipFile (outputZip);
//        DBG ("Zip file: " << outputZipFile.getNumEntries() << " entries");
        
        ZipFile zipFile(is);
        File moduleFolder (File::getCurrentWorkingDirectory().getChildFile ("jpm_modules"));
        moduleFolder.createDirectory();
        
        File target (moduleFolder.getChildFile (moduleId));
        DBG (target.getFullPathName());
        
        DBG (zipFile.getNumEntries() << " entries");
        Result result = zipFile.uncompressTo (target, true);
        
        Module module;
        module.setName (moduleId);
        module.setPath (target.getRelativePathFrom (File::getCurrentWorkingDirectory()));
        module.setSource ("Database");
        // TODO add repository etc...
        config.addModule (module);

        //rebuildjucer(); // this clobbered all the juce modules so:
        addModuleToJucer (module);
    }

    /**
     * Add a module that's already on the users local drive. 
     */
    void addLocalModule (const String& userSuppliedPath)
    {
        File folder (File::getCurrentWorkingDirectory().getChildFile (userSuppliedPath));

        if (! folder.exists())
        {
            printError ("module " + folder.getFullPathName() + " not found");
            return;
        }

        Module m;
        m.setSource ("LocalPath");
        m.setName (folder.getFileNameWithoutExtension());
        m.setVersion ("NA");
        /* A path provided by the user is made relative to the project, and without the module
         * folder itself being included to avoid confusing the introjucer. */
        m.setPath (folder.getParentDirectory().getRelativePathFrom (File::getCurrentWorkingDirectory()));
        config.addModule (m);

        rebuildjucer();
    }

    void addModuleToJucer (Module module)
    {
        openJucerFile();

        if (module.getSource() == "LocalPath" || module.getSource() == "Database")
            jucer->addModule (module.getName(), module.getPath());
        else
            jucer->addModule (module.getName());
    }


    /** Installs any modules that are missing from the jpm_modules folder. */
    void installMissingModules()
    {
        auto allModules = config.getModules();

        for (auto module : allModules)
        {
            auto jpmModulesFolder = File::getCurrentWorkingDirectory().getChildFile ("jpm_modules");
            auto name = module.getName();

            if (!jpmModulesFolder.getChildFile (name).exists())
            {
                printInfo("installing: " + name);
                module.install (jpmModulesFolder);
            }
        }
    }


    /* Command-line command handlers:- */

    void erasecache()
    {
        DownloadCache().clearCache();
    }

    void rebuildjucer()
    {
        openJucerFile();

        jucer->clearModules();

        auto allModules = config.getModules();

        for (auto m : allModules)
            addModuleToJucer (m);
    }

    void install()
    {
        if (commandLine.size() == 0)
        {
            printInfo ("installing missing modules");
            printInfo ("to force a refresh delete the jpm_modules folder first)");
            installMissingModules();
        }
        else
        {
            while (commandLine.size() > 0)
            {
                installModule (commandLine[0]);
                commandLine.remove (0);
            }
        }
    }

    void genmodule()
    {
        ModuleGenerator generator;
        generator.run (commandLine);
    }

    void adduser()
    {
        using namespace std;
        string username, password, email;
        
        cout << "Please enter a username: ";
        cin >> username;
        cout << "Please enter a password: ";
        cin >> password;
        cout << "Please enter your email: ";
        cin >> email;
        
        Database db;
        db.addUser (username, password, email);
        
        cout << "Successfully added " << username << " to database\n";
        
        UserConfig userConfig;
        userConfig.saveUser (username, password, email);
        
    }
    
    void publish()
    {
        File cwd = File::getCurrentWorkingDirectory();
        DBG ("got cwd");
        File moduleInfoFile = cwd.getChildFile ("juce_module_info");
        if (!moduleInfoFile.existsAsFile())
        {
            moduleInfoFile = cwd.getChildFile ("juce_module_info.json");
            if (!moduleInfoFile.existsAsFile())
            {
                JpmFatalExcepton("Can't find module info file in current directory", cwd.getFullPathName());
            }
        }
        DBG ("got juce_module_info");
        var moduleInfo;
        Result result = JSON::parse (moduleInfoFile.loadFileAsString(),  moduleInfo);
        
        if (result.wasOk())
        {
            UserConfig userConfig;
            Database db (userConfig.getProperties()["username"], userConfig.getProperties()["password"]);
            db.publish (moduleInfo);
        }
        else
        {
            JpmFatalExcepton ("error parsing module info file", result.getErrorMessage());
        }
    }
    
    void list()
    {
        Directory directory;
        String searchString;

        if (commandLine.size() == 0)
            searchString = "*";
        else
            searchString = commandLine[0];

        DBG ("searching for " << searchString);
        auto result = directory.getModulesByName (searchString);

        for (auto r : result)
            printInfo(r.getSummaryString());
    }

    void add()
    {
        if (commandLine.size() == 0)
            printError ("You need to specify a folder with a module in it");
        else
            addLocalModule (commandLine[0]);
    }

    ConfigFile config;
    ScopedPointer<JucerFile> jucer;
    Database database;

    StringArray commandLine;
};


