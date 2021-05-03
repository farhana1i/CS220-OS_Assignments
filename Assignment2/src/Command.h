#include<iostream>
#include "Helper.h"

const char PIPE_SYMBOL[] = "|";
const char FILE_INPUT_SYMBOL[] = "<";
const char FILE_OUTPUT_SYMBOL[] = ">";
const char FILE_IO_SYMBOL[] = "<>";
const char SIMPLE_COMMAND[] = "---";
const char WHITESPACES[] = " \t\n\r";


// actual command and its parameters
struct Command
{
    std::vector<char*> argv;
    std::string command_type;       // command type i.e simple, file input, file output, file input/output

    Command()
    {
        this->command_type = std::string(SIMPLE_COMMAND);           // nullptr represents that command is of simple type
    }

    Command(char* inputCommand)
    {
        std::string commandStr(inputCommand);
        if(Helper::trim_copy(commandStr).length() < 1){
            return;
        }
        this->setCommandType(commandStr);

        // simple command
        if(this->isFileInputCommand() || this->isFileOutputCommand() || this->isFileIOCommand())
        {
            std::vector<char*> fileCommandList;
            if(this->isFileIOCommand())
            {            
                Helper::parseOnDelim(inputCommand, fileCommandList, FILE_INPUT_SYMBOL);
                
                char* file_copy = fileCommandList.back();
                fileCommandList.pop_back();
                Helper::parseOnDelim(file_copy, fileCommandList, FILE_OUTPUT_SYMBOL);
                Helper::parseOnDelim(fileCommandList[0], this->argv, WHITESPACES);       // parse command
                
                char* inputFile = new char[strlen(fileCommandList[1]) + 1];
                strcpy(inputFile, fileCommandList[1]);
                char* outputFile = new char[strlen(fileCommandList[2]) + 1];
                strcpy(outputFile, fileCommandList[2]);

                this->argv.push_back(inputFile);       // input filename
                this->argv.push_back(outputFile);       // output filename

                if(file_copy) delete[] file_copy;
            }
            else if(this->isFileInputCommand())
            {
                Helper::parseOnDelim(inputCommand, fileCommandList, FILE_INPUT_SYMBOL);
                Helper::parseOnDelim(fileCommandList[0], this->argv, WHITESPACES);       // parse command

                char* inputFile = new char[strlen(fileCommandList[1]) + 1];
                strcpy(inputFile, fileCommandList[1]);
                this->argv.push_back(inputFile);       // input filename

            }
            else{
                Helper::parseOnDelim(inputCommand, fileCommandList, FILE_OUTPUT_SYMBOL);
                Helper::parseOnDelim(fileCommandList[0], this->argv, WHITESPACES);       // parse command

                char* outputFile = new char[strlen(fileCommandList[1]) + 1];
                strcpy(outputFile, fileCommandList[1]);
                this->argv.push_back(outputFile);       // output filename

            }
            for(auto str: fileCommandList){
                if(str) delete[] str;                
            }

        }
        else{
            Helper::parseOnDelim(inputCommand, this->argv, WHITESPACES);  // parse command
        }
    }

    Command(const Command& cmd)
    {
        this->command_type = cmd.command_type;
        for(auto str: cmd.argv)
        {
            char* temp = new char[strlen(str) + 1];
            strcpy(temp, str);
            this->argv.push_back(temp);
        }
    }

    // set command type
    void setCommandType(std::string commandStr)
    {
        // check either command contains input file symbol or not
        bool isInputFileCommand = commandStr.find(FILE_INPUT_SYMBOL) != std::string::npos ? true : false;
        // check either command contains output file symbol or not
        bool isOutputFileCommand = commandStr.find(FILE_OUTPUT_SYMBOL) != std::string::npos ? true : false;

        if(isOutputFileCommand && isInputFileCommand)
        {
            this->command_type = std::string(FILE_IO_SYMBOL);
        }
        else if(isInputFileCommand)
        {
            this->command_type = std::string(FILE_INPUT_SYMBOL);
        }
        else if(isOutputFileCommand)
        {
            this->command_type = std::string(FILE_OUTPUT_SYMBOL);
        }
        else{
            this->command_type = std::string(SIMPLE_COMMAND);
        }
    }

    // return true, if input command contains '<' symbol (file input)
    bool isFileInputCommand()
    {
        return (this->command_type ==  std::string(FILE_INPUT_SYMBOL));
    }

    // return true, if input command contains '>' symbol (file output)
    bool isFileOutputCommand()
    {
        return (this->command_type == std::string(FILE_OUTPUT_SYMBOL));
    }

    // return true, if input command contains '>' and ',' symbol (both file input/output)
    bool isFileIOCommand()
    {
        return (this->command_type == std::string(FILE_IO_SYMBOL));
    }


    // return true, if input command contains whitespaces only
    bool isEmptyCommand()
    {
        return (this->argv.empty() || (!this->argv[0]) || (std::string(this->argv[0]).find_first_not_of(WHITESPACES) == std::string::npos));
    }

    // return true, if input command is exit command
    bool isExitCommand()
    {
        return (this->argv[0]) && (Helper::trim_copy(std::string(this->argv[0])) == "exit");
    }

    // destructor
    ~Command()
    {
      for(auto str: argv)
      {
        if(str) delete[] str;
      }  
    }
};
