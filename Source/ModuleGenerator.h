#ifndef MODULEGENERATOR_H_INCLUDED
#define MODULEGENERATOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Utilities.h"


#define ID(x) const Identifier x (#x);

namespace ModuleIds
{
ID (id)
ID (name)
ID (version)
ID (description)
ID (website)
ID (license)
ID (dependencies)
ID (include)
ID (compile)
ID (browse)

ID (file)
ID (target)

ID (OSXFrameworks)
ID (iOSFrameworks)
ID (LinuxLibs)
ID (mingwLibs)
};
#undef ID

/**
 We need to provide custom headers to github to use this beta service.
 */
String readEntireTextStreamCustomHeaders (URL url)
{
    String headers = "Accept: application/vnd.github.drax-preview+json";
    const ScopedPointer<InputStream> in (url.createInputStream (false, nullptr, nullptr, headers));

    if (in != nullptr)
        return in->readEntireStreamAsString();

    return String();
}


class GitHubLicenseApi
{
public:
    static String getText (const String& licenseCode)
    {
        URL url ("https://api.github.com/licenses/" + licenseCode);
        auto text = readEntireTextStreamCustomHeaders (url);
        var response = JSON::fromString (text);
        return response["body"];
    }
};

/**
 The user will want to do something like:
   jpm genmodule <folder_name> <namespace>
 */
class ModuleGenerator
{
public:
    ModuleGenerator() {}

    void writeJuceModuleInfo()
    {
        auto name = folder.getFileName();
        auto json = new DynamicObject();
        json->setProperty (ModuleIds::id, name);
        json->setProperty (ModuleIds::name, name);
        json->setProperty (ModuleIds::version, "0.0.1");
        json->setProperty (ModuleIds::description, "Add description");
        json->setProperty (ModuleIds::website, "Add URL");
        json->setProperty (ModuleIds::license, "Not Specified");

        /* Dependencies. */
        Array<var> deps;

        for (auto d : dependencies)
        {
            auto depInfo = new DynamicObject();
            depInfo->setProperty (ModuleIds::id, d);
            depInfo->setProperty (ModuleIds::version, "matching"); /* I don't know what this actually does! */
            deps.add (depInfo);
        }

        json->setProperty (ModuleIds::dependencies, deps);

        /* Include. */
        json->setProperty (ModuleIds::include, name + ".h");

        /* Compile */
        Array<var> compile;
        {
            auto c = new DynamicObject();
            c->setProperty (ModuleIds::file, name + ".cpp");
            c->setProperty (ModuleIds::target, "! xcode");
            compile.add (c);
        }

        {
            auto c = new DynamicObject();
            c->setProperty (ModuleIds::file, name + ".mm");
            c->setProperty (ModuleIds::target, "xcode");
            compile.add (c);
        }

        json->setProperty (ModuleIds::compile, compile);
        /* Browse */
        {
            Array<var> browse;
            Array<File> directories;
            folder.findChildFiles (directories, File::findDirectories, true, "*");

            for (auto f : directories)
                browse.add (f.getRelativePathFrom (folder) + "/*");

            json->setProperty (ModuleIds::browse, browse);
        }

        /* Blank frameworks and libraries requirements */

        /* Write it. */
        File output = folder.getChildFile ("juce_module_info");
        output.replaceWithText (JSON::toString (var (json)));
    }

    void writeSources()
    {
        auto header = folder.getChildFile (folder.getFileName() + ".h");
        auto cpp = folder.getChildFile (folder.getFileName() + ".cpp");
        auto mm = folder.getChildFile (folder.getFileName() + ".mm");

        header.replaceWithText (getHeader());
        cpp.replaceWithText (getSourceCpp());
        mm.replaceWithText (getSourceObjectiveC());
    }

    void run (const StringArray& commandLine)
    {
        if (commandLine.size() < 2)
        {
            usage();
            return;
        }

        folder = File (File::getCurrentWorkingDirectory().getChildFile (commandLine[0]));
        namespaceName = commandLine[1];

        for (int i = 2; i < commandLine.size(); ++i)
            dependencies.add (commandLine[i]);

        findSourceFiles (folder);

        printInfo ("Source Files");

        for (auto s : sourceFiles)
            printInfo (s);

        printInfo ("Header Files");

        for (auto h : headerFiles)
            printInfo (h);

        writeSources();
        writeJuceModuleInfo();
    }

    void usage()
    {
        String usage =
            "\n"
            "genmodule <source_folder> <namespace> [<required dependencies...>]" "\n"
            "\n"
            "\n"
            "A juce_module_info file will be added to the source_folder." "\n"
            "The source_folder name will be used as the module name." "\n"
            "source_folder.h source_folder.mm source_folder.cpp files will" "\n"
            "be created in that folder overwriting any previous versions" "\n"
            "\n";

        std::cerr << usage;
    }


    void findSourceFiles (File& rootFolder)
    {
        Array<File> headers;
        rootFolder.findChildFiles (headers, File::findFilesAndDirectories, true, "*.h");

        for (auto f : headers)
        {
            auto path = f.getRelativePathFrom (rootFolder);

            /* avoid recursively including our own master header. */
            if (path != rootFolder.getFileName() + ".h")
                headerFiles.add (path);
        }

        Array<File> sources;
        rootFolder.findChildFiles (sources, File::findFilesAndDirectories, true, "*.cpp");
        rootFolder.findChildFiles (sources, File::findFilesAndDirectories, true, "*.mm");

        for (auto f : sources)
        {
            auto path = f.getRelativePathFrom (rootFolder);

            /* avoid recursively including our own master source files. */
            if (
                path != rootFolder.getFileName() + ".cpp"
                &&
                path != rootFolder.getFileName() + ".mm"
            )
                sourceFiles.add (path);
        }
    }

    /* Master header file generation. */
    void appendHeaderGuard (String& t)
    {
        t += "#pragma once"; /* This could be better ... */
        t += "\n";
    }

    void appendDependencies (String& t)
    {
        for (auto d : dependencies)
        {
            String includeFile = "modules/" + d + "/" + d + ".h";
            t += "#include <" + includeFile + ">\n";
        }
    }

    void appendNamespaceStart (String& t)
    {
        t += "\n";
        t += "namespace " + namespaceName + " {"; /* This could be better ... */
        t += "\n";
    }

    void appendIncludes (String& t, StringArray includes)
    {
        t += "\n";

        for (auto header : includes)
        {
            t += "#include \"" + header + "\"";
            t += "\n";
        }

        t += "\n";
    }

    void appendLicenseComment (String& t)
    {
        t += "\n";
        t += "/*";
        t += "\n";
        t += GitHubLicenseApi::getText ("mit");
        t += "\n";
        t += "*/";
        t += "\n";
        t += "\n";
    }

    String getHeader()
    {
        String t;
        appendLicenseComment (t);
        appendHeaderGuard (t);
        appendDependencies (t);
        appendNamespaceStart (t);
        appendIncludes (t, headerFiles);

        t += "}\n"; /* end namespace */
        t += "\n";
        return t;
    }

    /*****/

    String getSourceCpp()
    {
        String t;

        StringArray includes;
        includes.add ("AppConfig.h");
        includes.add (folder.getFileName() + ".h");

        appendLicenseComment (t);
        appendIncludes (t, includes);
        appendNamespaceStart (t);
        appendIncludes (t, sourceFiles);

        t += "}\n"; /* end namespace */
        t += "\n";
        return t;
    }

    /*****/

    String getSourceObjectiveC()
    {
        String t;
        StringArray includes (folder.getFileName() + ".cpp");
        appendIncludes (t, includes);
        return t;
    }


private:
    StringArray dependencies;
    StringArray headerFiles;
    StringArray sourceFiles;
    String namespaceName;
    File folder;
};


#endif  // CONFIGFILE_H_INCLUDED
