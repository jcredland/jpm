# JUCE Package Manager

Install and manage JUCE modules from the command line in style!

`jpm install juce_*`

Find JUCE modules you didn't know about: 

`jpm list`

Create JUCE modules with as little fuss as possible:

`jpm genmodule myModuleFolder myNamespace juce_core`

Include a local module:

`jpm add ./local_modules/my_existing_module`

## Submitting modules

If there end up being LOTS of modules we'll have to automate this.  But for now submit and ISSUE on github or a pull request.  

## Goals

* Simplicity
* Ensure repeatable builds
* Encourage sharing of code
* Make it easier to reuse your own code

AND 

* Avoid the headaches of git submodule as much as possible

## TODO 

Lots.  It'd be good if it could install the introjucer, yes? :) 

# Full Usage

     $ jpm
    JUCE v4.0.2

    jpm - a juce package manager

    ESSENTIAL COMMANDS
    jpm install <source>      add and install a modules
    jpm install               download any missing modules for the current project
    jpm add <source>          add a local module without using the directory
    jpm list [<wildcard>]     show all available modules, e.g. jpm list *core*
    jpm erasecache            erase the download cache

    OTHER COMMANDS
    jpm genmodule <name>      create a module template [ not yet implemented ]
    jpm rebuildjucer          rewrite the modules section of the jucer file

    Run this from the root of your JUCE project
