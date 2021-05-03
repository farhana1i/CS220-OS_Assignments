#include "CommandList.h"

struct Shell
{
    // control the whole flow of shell
    void shellController()
    {
        bool isExit = false;
        while (!isExit)
        {
            printCWD();
            std::string inputCommand;
            getline(std::cin, inputCommand);            // input command from the user

            CommandList cmdList(inputCommand);
            isExit = cmdList.execute(inputCommand);
        }
    }
    
    
    // to print current working directory
    void printCWD()
    {
        std::string user = getenv("USER");
        std::string directory = get_current_dir_name();

        int ind = directory.find(user);
        if(ind != std::string::npos)
        {
            directory = directory.substr(ind+user.length());
        }
        
        std::cout<<user<<"@ubuntu:~"<<directory<<"$ ";
    }

};