#include <iostream>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <vector>
#include <unistd.h>
#include <string.h>

// Helper Class For Helping Functions
struct Helper
{
    // trim from start (in place)
    static inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
    }

    // trim from end (in place)
    static inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    // trim from both ends (in place)
    static inline void trim(std::string &s) {
        ltrim(s);
        rtrim(s);
    }

    // trim from start (copying)
    static inline std::string ltrim_copy(std::string s) {
        ltrim(s);
        return s;
    }

    // trim from end (copying)
    static inline std::string rtrim_copy(std::string s) {
        rtrim(s);
        return s;
    }

    // trim from both ends (copying)
    static inline std::string trim_copy(std::string s) {
        trim(s);
        return s;
    }


    // parse str w.r.t delim
    static void parseOnDelim(char* str, std::vector<char*>& list, const char* delim)
    {
        char* token = strtok(str, delim);
        while(token)
        {
            // remove all whitespaces
            std::string temp(token);
            token = strdup(Helper::trim_copy(temp).c_str());
            
            list.push_back(token);
            token = strtok(NULL, delim);
        }
    }



    // convert vector<char*> into char**
    static char** vectorToChar(std::vector<char*> vect)
    {
        const int SIZE = vect.size();
        char** result = new char*[SIZE];
        for(unsigned int i = 0; i < SIZE; ++i)
        {
            result[i] = vect.at(i);
        }
        return result;
    }


    // read data using file descriptor
    static char* readUsingDescriptror(int fd)
    {
        std::string inputData;          // to store file data
        char ch;
        while(read(fd, &ch, 1))
        {
            inputData.push_back(ch);
        }
        inputData.push_back('\0');              // place null at the last of data
        return strdup(inputData.c_str());       // string to char*
    }
};


