#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Command.h"

class CommandList
{
    // backup file descriptors for standard input output i.e std::cin and std::cout
    int readBackup;
    int writeBackup;
    bool isPipedCommand;            // falg to check either command is piped or not
    std::vector<Command*> commandList;

    void __init__()
    {
        this->readBackup = dup(0);
        this->writeBackup = dup(1);   
        this->isPipedCommand = false;
    }
public:
    CommandList()
    {
        this->__init__();
    }

    CommandList (const std::string& inputCommand)
    {
        this->__init__();        
        this->isPipedCommand = inputCommand.find(PIPE_SYMBOL) != std::string::npos ? true : false;

        // parse input command line 
        std::vector<char*> tempCommandList;

        if(isPipedCommand){
            Helper::parseOnDelim(strdup(inputCommand.c_str()), tempCommandList, PIPE_SYMBOL);
        }
        else{
            tempCommandList.push_back(strdup(Helper::trim_copy(inputCommand).c_str()));
        }
        
        for(auto str : tempCommandList)
        {
            this->commandList.push_back(new Command(str));
        }
    }


    // execute input command
    bool execute(std::string inputCommand)
    {
        // for pipes
        if(this->isPipedCommand)
        {   
            // create pipes
            const int TOTAL_PIPES = this->commandList.size();
            int pipesList[TOTAL_PIPES][2];
            for(unsigned int i = 0; i < TOTAL_PIPES; ++i)
            {   
                pipe(pipesList[i]);
                Command* cmd = this->commandList[i];

                if(cmd->isEmptyCommand()) continue;
                if(cmd->isExitCommand()) return true;

                // command contains both file input and output symbols
                if(cmd->isFileIOCommand())
                {                 
                    char* outputFileName = cmd->argv.back();
                    cmd->argv.pop_back();
                    char* inputFileName = cmd->argv.back();
                    cmd->argv.pop_back();
                    cmd->argv.push_back(NULL);

                    int fd = open(inputFileName, O_RDONLY);
                    if(fd == -1) continue;
                    executePipeCommand(cmd, pipesList, i, TOTAL_PIPES, FILE_INPUT_SYMBOL, fd);
                    close(fd);

                    fd = open(outputFileName, O_CREAT | O_WRONLY, 0666);
                    if(fd == -1) continue;

                    //read data from pipe and write in the output file
                    char* pipeData = Helper::readUsingDescriptror(pipesList[i][0]);
                    dup2(fd, 1);
                    std::cout<<pipeData;
                    close(fd);
                    
                    recoverIO();

                    //deallocate memory
                    if(outputFileName) delete[] outputFileName;
                    if(inputFileName) delete[] inputFileName;
                    if(pipeData) delete[] pipeData;
                }
                else if(cmd->isFileInputCommand())
                {                    
                    char* inputFileName = cmd->argv.back();
                    cmd->argv.pop_back();
                    cmd->argv.push_back(NULL);

                    int fd = open(inputFileName, O_RDONLY);
                    if(fd == -1) continue;
                    executePipeCommand(cmd, pipesList, i, TOTAL_PIPES, FILE_INPUT_SYMBOL, fd);

                    //deallocate memory
                    if(inputFileName) delete[] inputFileName;
                }
                else if(cmd->isFileOutputCommand())
                {
                    char* outputFileName = cmd->argv.back();
                    cmd->argv.pop_back();
                    cmd->argv.push_back(NULL);

                    int fd = open(outputFileName, O_WRONLY | O_CREAT, 0666);
                    if(fd == -1) continue;
                    executePipeCommand(cmd, pipesList, i, TOTAL_PIPES, FILE_OUTPUT_SYMBOL, fd);

                    //deallocate memory
                    if(outputFileName) delete[] outputFileName;
                }
                else
                {
                    cmd->argv.push_back(NULL); 
                    executePipeCommand(cmd, pipesList, i, TOTAL_PIPES, PIPE_SYMBOL);
                }
           }

        }
        else
        {
            for(auto cmd: this->commandList)
            {   
                if(cmd->isEmptyCommand()) continue;
                if(cmd->isExitCommand()) return true;

                if(cmd->isFileIOCommand())
                {
                    char* outputFileName = cmd->argv.back();
                    cmd->argv.pop_back();
                    char* inputFileName = cmd->argv.back();
                    cmd->argv.pop_back();
                    cmd->argv.push_back(NULL);
                    
                    // open input and output files
                    int read_fd = open(inputFileName, O_RDONLY);
                    int write_fd = open(outputFileName, O_CREAT | O_WRONLY, 0666);
                    if(read_fd == -1 || write_fd == -1)
                    {
                        perror(NULL);
                        continue;
                    }
                    dup2(read_fd, 0);
                    dup2(write_fd, 1);

                    executeSimpleCommand(cmd);
                    close(read_fd);
                    close(write_fd);
                    recoverIO();
                    //deallocate memory
                    if(outputFileName) delete[] outputFileName;
                    if(inputFileName) delete[] inputFileName;
                }
                else if(cmd->isFileInputCommand())
                {
                    char* inputFileName = cmd->argv.back();
                    cmd->argv.pop_back();
                    cmd->argv.push_back(NULL);

                    // open input and output files
                    int read_fd = open(inputFileName, O_RDONLY);
                    if(read_fd == -1)
                    {
                        perror(NULL);
                        continue;
                    }
                    dup2(read_fd, 0);
                    executeSimpleCommand(cmd);
                    close(read_fd);
                    recoverIO();

                    //deallocate memory
                    if(inputFileName) delete[] inputFileName;
                }
                else if(cmd->isFileOutputCommand())
                {
                    char* outputFileName = cmd->argv.back();
                    cmd->argv.pop_back();
                    cmd->argv.push_back(NULL);
                    
                    // open input and output files
                    int write_fd = open(outputFileName, O_CREAT | O_WRONLY, 0666);
                    if(write_fd == -1)
                    {
                        perror(NULL);
                        continue;
                    }
                    dup2(write_fd, 1);

                    executeSimpleCommand(cmd);
                    close(write_fd);
                    recoverIO();

                    //deallocate memory
                    if(outputFileName) delete[] outputFileName;
                }
                else{

                    cmd->argv.push_back(NULL); 
                    executeSimpleCommand(cmd);
                }
            }
        }
        return false;
    }

    // to execute a simple command
    void executeSimpleCommand(Command*& cmd)
    {
        pid_t status = fork();
        if(status == 0)
        {
            if(execvp(cmd->argv[0], Helper::vectorToChar(cmd->argv)) == -1)
            {
                perror(NULL);
                exit(0);
            }    
        }
        else if(status > 0)
        {
            wait(NULL);
        }
        else{
            perror(NULL);
        }
    }

    // recover standard I/O
    void recoverIO()
    {
        dup2(writeBackup, 1);
        dup2(readBackup, 0);
    }

    // redirect input for '<' symbol (input from file)
    void redirectIOForFileInput(unsigned int currPipeNum, int pipesList[][2], int fileDescriptor)
    {
        if(currPipeNum == 0)
        {
            dup2(fileDescriptor, 0);
        }
        else
        {
            dup2(pipesList[currPipeNum][1], 1);
            dup2(fileDescriptor, 0);
        }
    }

    // redirect input for '>' symbol (output to file)
    void redirectIOForFileOuput(unsigned int currPipeNum, int pipesList[][2],  int fileDescriptor)
    {
        if(currPipeNum == 0)
        {
            dup2(fileDescriptor, 1);
        }
        else
        {
            dup2(pipesList[currPipeNum - 1][0], 0);
            dup2(fileDescriptor, 1);
        }
    }


    // redirect input for '|' symbol (pipes)
    void redirectIOForPipe(int currPipeNum, int pipesList[][2], const int TOTAL_PIPES)
    {
        // redirect I/O
        if(currPipeNum == 0)                        // no need to redirect read-end
        {
            dup2(pipesList[currPipeNum][1], 1);
        }
        else if(currPipeNum == TOTAL_PIPES - 1)     // no need to redirect write-end
        {
            dup2(pipesList[currPipeNum - 1][0], 0);
        }
        else
        {
            dup2(pipesList[currPipeNum - 1][0], 0);
            dup2(pipesList[currPipeNum][1], 1);
        }
    }

    // execute pipe commands
    // file descriptor is needed in case of file only
    void executePipeCommand(Command*& cmd, int pipesList[][2], int currPipeNum, const int TOTAL_PIPES, const char* command_type, int fileDescriptor = -1)
    {
        pid_t status = fork();            // create a new process
        
        // child process
        if(status == 0)
        {
            if(!strcmp(command_type, FILE_INPUT_SYMBOL))
            {
                redirectIOForFileInput(currPipeNum, pipesList, fileDescriptor);
            }
            else if(!strcmp(command_type, FILE_OUTPUT_SYMBOL))
            {
                redirectIOForFileOuput(currPipeNum, pipesList, fileDescriptor);
            }
            redirectIOForPipe(currPipeNum, pipesList, TOTAL_PIPES);

            // fail to execute commmand
            if(execvp(cmd->argv[0], Helper::vectorToChar(cmd->argv)) == -1)
            {
                perror(NULL);
                return;
            }
        }
        // parent process
        else if(status > 0)
        {
            wait(NULL);                 // wait for child process to terminate
            if (currPipeNum == TOTAL_PIPES - 1){
                dup2(writeBackup, 1);
            }
            close(pipesList[currPipeNum][1]);
        }
        // fork() fails
        else
        {
            perror(NULL);
            return;
        }
    }

    ~CommandList()
    {
        dup2(writeBackup, 1);
        close(writeBackup);
        dup2(readBackup, 0);
        close(readBackup);

        for(auto cmd: this->commandList)
        {
            if(cmd) delete cmd;
        }

    }
};
