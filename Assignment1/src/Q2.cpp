#include<iostream>
#include<string.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<sys/wait.h>



// read data using file descriptor
// isUpdate flag is false, if we want to only read file
// isUpdate flag is true, if we want to both read file and update data simultaneously
// [update data i.e remove numbers and change cases of alphabets]
char* readUsingDescriptror(int fd, unsigned int& dataSize, bool isUpdateON = false)
{
    std::string inputData;          // to store file data
    char ch;
    while(read(fd, &ch, 1))
    {
        // update data
        if(isUpdateON)
        {
            if(ch >= 97 && ch <= 122)
            {
                ch -= 32;
            }
            else if(ch >= 65 && ch <= 90)
            {
                ch += 32;   
            }
        }

        if(!isUpdateON || (isUpdateON && (ch < '0' || ch > '9')))
        {
            inputData.push_back(ch);
            ++dataSize;
        }
    }
    inputData.push_back('\0');              // place null at the last of data
    return strdup(inputData.c_str());       // string to char* conversion
}



int main(int argc, char** argv)
{
    // file descriptors
    int fd1[2];                     // parent reads and child writes      
    int fd2[2];                     // child reads and parent writes

    
    // check either user has passed both filenames in arguement or not
    if(argc < 3)
    {
        std::cout<<"Invalid Input! Please Enter File Names As well!\n";
        return 0;
    }


    char* inputFile = argv[1];      // load input file name
    char* outputFile = argv[2];     // load output file name

    // fail to open pipe(s)
    if(pipe(fd1) == -1 || pipe(fd2) == -1)
    {
        perror(NULL);
        return 0;
    }

    int p_id = fork();          // create a new process

    // child process
    if(p_id == 0)
    {
        close(fd1[1]);          // child process reads from pipe1 
        // read and update data from pipe1 simultaneously
        unsigned int dataSize = 0;
        char* pipeData = readUsingDescriptror(fd1[0], dataSize, true);
        close(fd1[0]);          // done with reading of pipe1
   
        
        close(fd2[0]);          // child process writes in pipe2
        // write in pipe2
        if(pipeData != nullptr)
        {
            write(fd2[1], pipeData, dataSize);
        }
        close(fd2[1]);          // done with writing in pipe2

        // deallocate memory
        if(pipeData != nullptr){
            delete[] pipeData;
        }
    }

    // parent process
    else if(p_id > 0)
    {
        // open file for reading
        int fd = open(inputFile, O_RDONLY, 0);
        if(fd == -1)
        {
            perror(NULL);
            return 0;
        }

        close(fd1[0]);          // parent process writes in pipe1
        // read data
        unsigned int dataSize = 0;
        char* inputData = readUsingDescriptror(fd, dataSize);
        // write in pipe
        if(inputData != nullptr)
        {
            write(fd1[1], inputData, dataSize);
        }
        close(fd1[1]);          // done with writing in pipe1

        // deallocate memory
        if(inputData != nullptr){
            delete[] inputData;
        }
        close(fd);

        // wait for child to complete, so that we can read updated data from the pipe
        wait(NULL);
        
        close(fd2[1]);          // parent process reads from pipe2
        // read from pipe2
        dataSize = 0;
        char* pipeData = readUsingDescriptror(fd2[0], dataSize);
        close(fd2[0]);          // done with reading from pipe2

        // write updated data into output file
        fd = open(outputFile, O_WRONLY | O_CREAT, S_IRWXU);
        if(pipeData != nullptr)
        {
            write(fd, pipeData, dataSize);
            delete[] pipeData;
        }
        close(fd);
    }
    
    // error
    else
    {
        perror(NULL);
    }

    return 0;
}
