/*
  ==============================================================================

    Module.h
    Created: 7 Dec 2015 1:32:17pm
    Author:  Jim

  ==============================================================================
*/

#ifndef MODULE_H_INCLUDED
#define MODULE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"


class DownloadedModuleCache
{
public:
	DownloadedModuleCache()
	{
		location = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("jpm.modulecache");
	}



	File location;
};

/** Refers to a module. */
class Module
{
public:
	Module() : state("module") {}

	Module(ValueTree & state_)
	{
		state = state_;

		if (! getValidSources().contains(getSource()))
			std::cerr << "error: invalid source: " << getSource() << std::endl;
	}

	ValueTree getStateAsValueTree() const { return state; }

	bool isValid() const { return getName().isNotEmpty(); }

	void downloadFromGitHub(const File & tempFolder)
	{
		std::cout << "Downloading" << std::endl;

		MemoryBlock memoryBlock;
		URL url(getPath() + getVersion() + ".zip");

		if (!url.readEntireBinaryStream(memoryBlock, false))
		{
			std::cout << "error: downloading " + getName() << std::endl;
			return;
		}

		MemoryInputStream inputStream(memoryBlock, false);


		std::cout << "Uncompressing to " << tempFolder.getFullPathName() << std::endl;
		ZipFile zip(inputStream);
		auto result = zip.uncompressTo(tempFolder, true); 

		if (result.failed())
			std::cout << result.getErrorMessage() << std::endl;	
	}

	/** Install this module into a destination folder. */
	void install(const File & destinationFolder)
	{
		File tempFolder = File::createTempFile(".jpm");

		if (getSource() == "GitHub")
			downloadFromGitHub(tempFolder);
		else
			std::cerr << "Invalid source " + getSource() << std::endl;

		auto sourceFile = tempFolder.getChildFile(getSubPath());

		if (! sourceFile.exists())
		{
			std::cerr << "Invalid module" << std::endl;
			return;
		}

		auto result = sourceFile.copyDirectoryTo(destinationFolder.getChildFile(getName()));

		if (!result)
		{
			std::cerr << "Problem copying module" << std::endl;
		}
	}

	/* Getters and setters. */
	String getName() const {
		return state["name"];
	}
	void setName(const String& name) {
		state.setProperty("name", name, nullptr);
	}
	String getVersion() const {
		return state["version"];
	}
	void setVersion(const String& version) {
		state.setProperty("version", version, nullptr);
	}
	String getSource() const {
		return state["source"];
	}
	void setSource(const String& source) {
		state.setProperty("source", source, nullptr);
	}
	String getPath() const {
		return state["path"];
	}
	void setPath(const String& path) {
		state.setProperty("path", path, nullptr);
	}
	String getSubPath() const {
		return state["subpath"];
	}
	void setSubPath(const String& subpath) {
		state.setProperty("subpath", subpath, nullptr);
	}

private:
	static StringArray & getValidSources() 
	{
		static StringArray validSources;
		validSources.add("GitHub"); 
		return validSources;
	}

	ValueTree state;
};




#endif  // MODULE_H_INCLUDED
