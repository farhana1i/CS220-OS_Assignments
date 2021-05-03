#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <vector>


// execute command
void executeCommand(char **argv)
{
    int p_id = fork();

    // child process
    if (p_id == 0)
    {
        int status = execvp(argv[0], argv);
        if (status == -1)
        {
            std::cout << "Fail To Open Command File!\n";
            return;
        }
    }

    // parent process
    else if (p_id > 0)
    {
        wait(NULL); // wait for child process to complete
    }

    // error
    else
    {
        std::cout << "Fail To Create New Process!\n";
    }
}


// tokenize user input command into command and arguements and return number of arguements
int tokenization(char* input_command, char**& argv)
{
    std::vector<char*> arguementsList;
    char word[200];
    unsigned int word_size = 0;

    for (unsigned int i = 0; i < strlen(input_command); ++i)
    {
        if (input_command[i] != ' ' && input_command[i] != '\t' && input_command[i] != '\n')
        {
            word[word_size++] = input_command[i];
        }

        if (input_command[i] == ' ' || input_command[i] == '\t' || input_command[i] == '\n' || input_command[i + 1] == '\0')
        {
            word[word_size] = '\0';
            char* wordPtr = new char[word_size + 1];
            strcpy(wordPtr, word);

            arguementsList.push_back(wordPtr);
            word_size = 0;
        }
    }

    // copy arguemnts list in tempArgv
    argv = new char* [int(arguementsList.size()) + 1];
    unsigned int i = 0;
    for (auto arg : arguementsList)
    {
        argv[i++] = arg;
    }
    argv[i] = NULL;

    return arguementsList.size();
}



// shell to control command input and response output
void shell()
{
    while (true)
    {
        char input_command[1000]; // to store user input command
        std::cout << ">>";
        std::cin.getline(input_command, 1000);

        char **argv = NULL;

        //  tokenize command and arguements
        unsigned int argc = tokenization(input_command, argv);

        if (argv != NULL && !strcmp(argv[0], "exit"))    // terminate shell
        {
            return;
        }

        // execute command
        executeCommand(argv);

        //deallocate memory
        for(unsigned int i = 0; i < argc; ++i)
        {
            if(argv[i] != nullptr)
            {
                delete[] argv[i];
            }
        }
        if(argv != nullptr) delete[] argv;
    }
}

int main()
{
    shell();
    return 0;
}
