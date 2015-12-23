#include "../JuceLibraryCode/JuceHeader.h"

#include "Main.h"
#include "Database.h"

class Main__Tests : public UnitTest
{
public:
    Main__Tests() : UnitTest("End-to-end tests") {}
    void runTest() override 
    {
        listCommand();
        generateSalt();
        stringToSHA1();
        //addUser();
    }
    
    void addUser()
    {
        // Add and delete a user
        //Database database;
        //String result = database.addUser("testUser", "password", "test@testuser.com");
        // Delete user
        
        // Call ConfigFile.saveUser and check file exists
    }
    
    void generateSalt()
    {
        Database database;
        beginTest ("generateSalt should generate a random 32 character hex string");
        StringArray saltArray;
        saltArray.add (database.generateSalt());
        saltArray.add (database.generateSalt());
        saltArray.add (database.generateSalt());
        
        for (auto salt : saltArray)
        {
            expect (salt.length() == 32);
        }
        expect (saltArray[0].compare (saltArray[1]) != 0);
        expect (saltArray[1].compare (saltArray[2]) != 0);
    }
    
    void stringToSHA1()
    {
        Database database;
        beginTest ("stringToSHA1 should generate a valid SHA1 hash from a given string");
        String password("somePassword1");
        String validSHA1Hash("eaced31083513e4c54a396176e95480d6d77e9cd");
        
        expect (database.stringToSHA1 (password).compare (validSHA1Hash) == 0);
    }
    
    void listCommand()
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
