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

#include "Main.h"

ScopedPointer<OutputRecorder> outputRecorder;

void usage()
{
    std::cout << std::endl;
    std::cout << "jpm - a juce package manager  (version " << Constants::versionString << ")" << std::endl;
    std::cout << std::endl;
    std::cout << "ESSENTIAL COMMANDS" << std::endl;
    std::cout << "jpm install <source>      add and install a module" << std::endl;
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
    std::cout << std::endl;
    std::cout << "Note: it will want to manage _all_ your modules and will replace" << std::endl;
    std::cout << "any module information already in your jucer file.  It will not" << std::endl;
    std::cout << "however overwrite any #define settings for modules you have set." << std::endl;

}


int main (int argc, char* argv[])
{
    StringArray commandLineArguments;

    for (int i = 0; i < argc; ++i)
        commandLineArguments.add (argv[i]);

    if (commandLineArguments.size() <= 1)
    {
        usage();
        return 1;
    }
    
    if (commandLineArguments.size() == 2 && commandLineArguments[1] == "--run-internal-tests")
    {
        UnitTestRunner testRunner;
        testRunner.runAllTests();
        return 0; 
    }

    /*
    This mega-try/catch probably isn't the best idea. Though it's a commandline failure and
    some hard-failure modes are probably not inappropriate.
    */
    try
    {
        App app (commandLineArguments);
        app.run();
    }
    catch (InvalidJucerFormat)
    {
        std::cerr << "exception: invalid jucer file format" << std::endl;
        return 1;
    }
    catch (JpmFatalExcepton e)
    {
        std::cerr << "exception: " << e.error << std::endl;
        std::cerr << "exception: " << e.debugInfo << std::endl;
        return 1;
    }

    return 0;
}


