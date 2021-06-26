#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <fstream>


// parameter for thread
struct paramter_t
{
    std::string filename;               // name of file from where to read data
};

// return value of thread
struct return_t
{
    unsigned int count;                 // number of elements
    long long int sum;                  // sum of elements

    return_t()
    {
        this->sum = this->count = 0;
    }
};

// to read data file and calculate its count and sum
void calculateResult(std::string filename, return_t* result)
{
    std::ifstream input_file(filename);
    if(input_file.fail()){
        std::cout<<"Fail to open "<<filename<<std::endl;
        return;
    }

    // calculate sum and count of the file data
    int num = 0;
    while(!input_file.eof())
    {
        input_file >> num;
        result->sum += num;
        ++(result->count);
    }
    input_file.close();
}

// thread to calculate sum and count
void* thread (void* p)
{
    paramter_t* paramtere = (paramter_t*)(p);
    return_t* result = new return_t();

    calculateResult(paramtere->filename, result);
    pthread_exit((void*) result);
}


int main(int argc, char** argv)
{
    // file names are not entered as command line arguement
    if(argc < 2){
        std::cout<<"Oops! You have missed files' name\n";
        exit(0);
    }

    const unsigned int TOTAL_THREADS = argc - 1;    // total files to read, so total threads
    paramter_t paramList[TOTAL_THREADS];            // parameters for threads
    pthread_t threads_idList[TOTAL_THREADS];        // to store ids of threads   

    // to create threads
    for(unsigned int i = 0; i < TOTAL_THREADS; ++i)
    {
        // set parameteres
        paramList[i].filename = std::string(argv[i + 1]);        
        if(pthread_create(&threads_idList[i], NULL, &(thread), &(paramList[i]))){
            std::cout<<"Fail to create thread with id: " << threads_idList[i]<<std::endl;
        }
    }

    long long int total_sum = 0;            // to store total sum
    unsigned int total_count = 0;           // to store total count
    // to join threads
    for(unsigned int i = 0; i < TOTAL_THREADS; ++i)
    {
        return_t* result = nullptr;
        if(pthread_join(threads_idList[i], (void **)&result)){
            std::cout<<"Fail to join thread with id: " << threads_idList[i]<<std::endl;
        }
        else{
            total_sum += result->sum;
            total_count += result->count;
        }
        if(result) delete result;
    }

    float average = (float)(total_sum) / (float) total_count;
    std::cout<<"Total Sum is = " << total_sum<<std::endl;
    std::cout<<"Total Count is = " << total_count<<std::endl;
    std::cout<<"Average is = "<< average << std::endl;

    return 0;
}
