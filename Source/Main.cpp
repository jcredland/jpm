

#include "../JuceLibraryCode/JuceHeader.h"
#include <iostream>


class JucerFile
{
public:
    JucerFile(File & file_) : file(file_) {}
    void addModule(const String & moduleName, const String & path)
    {

    }
};


int main (int argc, char* argv[])
{
    std::cout << "jpm - a juce package manager" << std::endl;

    std::cout << "jpm install <location>: install a package" << std::endl;




    return 0;
}
