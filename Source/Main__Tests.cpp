#include "../JuceLibraryCode/JuceHeader.h"

#include "Main.h"

class Main__Tests : public UnitTest
{
    public:
        Main__Tests() : UnitTest("End-to-end tests") {}
        void runTest() override 
        {
            /* Enable output recording. */
            outputRecorder = new OutputRecorder(); 

            /* Initiate an instance of our application. */
            StringArray commandLine { "jpm", "list" };
            App app(commandLine);

            beginTest("list should return at least some basic JUCE modules"); 
            app.run(); 
            expect(outputRecorder->hasLineContaining("juce_core"));

            outputRecorder = nullptr;
        }
};


static Main__Tests main__tests;
