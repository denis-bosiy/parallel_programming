#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include "EasyBmp.h"
#include <vector>
#include <chrono>

enum THREAD_PRIORITY
{
    BELOW_NORMAL = -1,
    NORMAL,
    ABOVE_NORMAL
};

struct Args
{
    char* inputFilePath;
    char* outputFilePath;
    int coresCount;
    int threadsCount;
    std::vector<THREAD_PRIORITY> threadPriorities;

    Args(char* _inputFilePath, char* _outputFilePath, int _threadsCount, int _coresCount, std::vector<THREAD_PRIORITY> _threadPriorities)
    {
        inputFilePath = _inputFilePath;
        outputFilePath = _outputFilePath;
        threadsCount = _threadsCount;
        coresCount = _coresCount;
        threadPriorities = _threadPriorities;
    }
};

double GetGaussianFunctionValue(int x)
{
    const float M_PI = 3.14;
    const float SIGMA_VALUE = 10;

    return (1 / (sqrt(M_PI * SIGMA_VALUE * SIGMA_VALUE))) * exp(-(x * x) / (2 * SIGMA_VALUE * SIGMA_VALUE));
}

struct ThreadInputData
{
    std::vector<RGBApixel*> imageRow;
    int threadNumber;
    std::chrono::steady_clock::time_point programExecutionStartTime;

    ThreadInputData(std::vector<RGBApixel*> _imageRow, int _threadNumber, std::chrono::steady_clock::time_point _programExecutionStartTime)
    {
        imageRow = _imageRow;
        threadNumber = _threadNumber;
        programExecutionStartTime = _programExecutionStartTime;
    }
};

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
    const ThreadInputData threadInputData = *(ThreadInputData*)(lpParam);
    const std::vector<RGBApixel*> imageRow = threadInputData.imageRow;
    const int threadNumber = threadInputData.threadNumber;
    const std::chrono::steady_clock::time_point programExecutionStartTime = threadInputData.programExecutionStartTime;

    // Gaussian blur algorithm
    for (int i = 0; i < imageRow.size(); i++)
    {
        RGBApixel* sumPixel = new RGBApixel();
        sumPixel->Red = 0;
        sumPixel->Green = 0;
        sumPixel->Blue = 0;
        sumPixel->Alpha = 0;

        float gaussianFunctionValuesSum = 0;
        for (int j = i - 10; j < i + 10; j++)
        {
            if (j < 0)
            {
                continue;
            }
            if (j >= imageRow.size())
            {
                break;
            }

            const double gaussianFunctionValue = GetGaussianFunctionValue(abs(i - j));
            gaussianFunctionValuesSum += gaussianFunctionValue;

            sumPixel->Red = sumPixel->Red + (int)(imageRow[j]->Red * gaussianFunctionValue) % 255;
            sumPixel->Green = sumPixel->Green + (int)(imageRow[j]->Green * gaussianFunctionValue) % 255;
            sumPixel->Blue = sumPixel->Blue + (int)(imageRow[j]->Blue * gaussianFunctionValue) % 255;
        }

        imageRow[i]->Red = sumPixel->Red / gaussianFunctionValuesSum;
        imageRow[i]->Green = sumPixel->Green / gaussianFunctionValuesSum;
        imageRow[i]->Blue = sumPixel->Blue / gaussianFunctionValuesSum;
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - programExecutionStartTime);
    std::cout << "Execution time: " + std::to_string(duration.count()) + "; Thread index: " + std::to_string(threadNumber) + ";\n";

    ExitThread(0);
}

bool IsHelpRequired(char* argv[])
{
    return strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "/?") == 0;
}

Args ProcessArgs(int argc, char* argv[])
{
    const int MAX_THREADS_COUNT = 16;
    const std::string BELOW_NORMAL_VALUE = "below_normal";
    const std::string NORMAL_VALUE = "normal";
    const std::string ABOVE_NORMAL_VALUE = "above_normal";
    const int MAX_CORES_COUNT = 4;

    if (argc == 2 && IsHelpRequired(argv))
    {
        std::cout << "Program arguments:" << std::endl;
        std::cout << "inputImagePath: string to path, where locates input image" << std::endl;
        std::cout << "outputImagePath: string to path, where locates output image" << std::endl;
        std::cout << "threadsCount: number of threads, that will be catched by the program" << std::endl;
        std::cout << "[threadsPriorities]: threads priorities separated by space. ThreadPriority can be: below_normal, normal, above_normal" << std::endl;
        std::cout << "coresCount: number of cores, that will be catched by the program" << std::endl;

        throw std::invalid_argument("Happy coding!");
    }

    if (argc - 1 < 5)
    {
        throw std::length_error("There should be at least 5 arguments: inputImagePath, outputImagePath, threadsCount, [threadsPriorities] and coresCount");
    }

    try
    {
        std::ifstream testStream;
        testStream.open(argv[1]);
    }
    catch (std::exception)
    {
        throw std::invalid_argument("Can not open input image");
    }

    const int threadsCount = std::stoi(argv[3]);
    if (threadsCount < 1 || threadsCount > MAX_THREADS_COUNT)
    {
        const std::string errorMsg = "Threads count should be less than " +
            std::to_string(MAX_THREADS_COUNT + 1) +
            " and more than 0";
        throw std::invalid_argument(errorMsg);
    }

    std::vector<THREAD_PRIORITY> threadPriorities = {};
    for (int i = 0; i < threadsCount; i++)
    {
        if (argv[4 + i] == BELOW_NORMAL_VALUE || argv[4 + i] == NORMAL_VALUE || argv[4 + i] == ABOVE_NORMAL_VALUE)
        {
            if (argv[4 + i] == BELOW_NORMAL_VALUE)
            {
                threadPriorities.push_back(BELOW_NORMAL);
            }
            if (argv[4 + i] == NORMAL_VALUE)
            {
                threadPriorities.push_back(NORMAL);
            }
            if (argv[4 + i] == ABOVE_NORMAL_VALUE)
            {
                threadPriorities.push_back(ABOVE_NORMAL);
            }
        }
        else
        {
            const std::string errorMsg = "Thread priority of the " + std::to_string(i + 1) + " thread name is incorrect";
            throw std::invalid_argument(errorMsg);
        }
    }

    const int coresCount = std::stoi(argv[4 + threadsCount]);
    if (coresCount < 1 || coresCount > MAX_CORES_COUNT)
    {
        const std::string errorMsg = "Cores count should be less than " +
            std::to_string(MAX_CORES_COUNT + 1) +
            " and more than 0";
        throw std::invalid_argument(errorMsg);
    }

    return *new Args(argv[1], argv[2], threadsCount, coresCount, threadPriorities);
}

void ClearThreads(HANDLE* handles, int threadsCount)
{
    for (int i = 0; i < threadsCount; i++)
    {
        CloseHandle(handles[i]);
    }
}

void ProcessRows(int coresCount, HANDLE* handles, std::vector<THREAD_PRIORITY> threadPriorities, const std::vector<std::vector<RGBApixel*>> imageRows, std::chrono::steady_clock::time_point programExecutionStartTime)
{
    for (int i = 0; i < imageRows.size(); i++)
    {
        std::vector<RGBApixel*>* imageRow = new std::vector<RGBApixel*>(0);
        for (int j = 0; j < imageRows[i].size(); j++)
        {
            (*imageRow).push_back(imageRows[i][j]);
        }
        ThreadInputData* threadInputData = new ThreadInputData(*imageRow, i+1, programExecutionStartTime);
        handles[i] = CreateThread(NULL, 0, &ThreadProc, threadInputData, CREATE_SUSPENDED, NULL);
        // Set cores count to the thread
        SetProcessAffinityMask(handles[i], static_cast<DWORD_PTR>(1 << static_cast<int>(pow(coresCount - 1, 2))));
        // Set priority to the thread
        SetThreadPriority(handles[i], threadPriorities[i]);
    }

    for (int i = 0; i < imageRows.size(); i++)
    {
        ResumeThread(handles[i]);
    }

    WaitForMultipleObjects(imageRows.size(), handles, true, INFINITE);
    ClearThreads(handles, imageRows.size());
}

std::vector<RGBApixel*> ReadRowFromFile(BMP File, int rowIndex)
{
    std::vector<RGBApixel*> row = {};

    for (int i = 0; i < File.TellWidth(); i++)
    {
        RGBApixel* pixelFromFile = File(i, rowIndex);
        RGBApixel* newPixel = new RGBApixel();

        newPixel->Red = pixelFromFile->Red;
        newPixel->Green = pixelFromFile->Green;
        newPixel->Blue = pixelFromFile->Blue;
        newPixel->Alpha = pixelFromFile->Alpha;

        row.push_back(newPixel);
    }

    return row;
}

void CreateOutputFile(BMP& inputFile, const char* outputFilePath, std::vector<std::vector<RGBApixel*>>& totalImageRows)
{
    for (int rowIndex = 0; rowIndex < inputFile.TellHeight(); rowIndex++)
    {
        for (int columnIndex = 0; columnIndex < inputFile.TellWidth(); columnIndex++)
        {
            inputFile(columnIndex, rowIndex)->Red = totalImageRows[rowIndex][columnIndex]->Red;
            inputFile(columnIndex, rowIndex)->Green = totalImageRows[rowIndex][columnIndex]->Green;
            inputFile(columnIndex, rowIndex)->Blue = totalImageRows[rowIndex][columnIndex]->Blue;
            inputFile(columnIndex, rowIndex)->Alpha = totalImageRows[rowIndex][columnIndex]->Alpha;
        }
    }
    inputFile.WriteToFile(outputFilePath);
}

std::vector<std::vector<RGBApixel*>> CreateBluredImage(BMP& inputFile,
    int threadsCount,
    std::vector<THREAD_PRIORITY> threadPriorities,
    const char* inputFilePath,
    int coresCount,
    std::chrono::steady_clock::time_point programExecutionStartTime
)
{
    std::vector<std::vector<RGBApixel*>> totalImageRows = {};
    HANDLE* handles = new HANDLE[threadsCount];
    inputFile.ReadFromFile(inputFilePath);

    for (int rowIndex = 0; rowIndex < inputFile.TellHeight(); rowIndex = rowIndex + threadsCount)
    {
        std::vector<std::vector<RGBApixel*>> imageRows = {};
        int imageRowsCount = threadsCount;
        if (rowIndex + threadsCount >= inputFile.TellHeight())
        {
            imageRowsCount = inputFile.TellHeight() - rowIndex;
        }
        for (int i = 0; i < imageRowsCount; i++)
        {
            imageRows.push_back(ReadRowFromFile(inputFile, rowIndex + i));
        }

        ProcessRows(coresCount, handles, threadPriorities, imageRows, programExecutionStartTime);

        for (int i = 0; i < imageRowsCount; i++)
        {
            totalImageRows.push_back(imageRows[i]);
        }
    }

    return totalImageRows;
}

int main(int argc, char* argv[])
{
    try
    {
        auto start = std::chrono::high_resolution_clock::now();
        Args parsedArgs = ProcessArgs(argc, argv);
        BMP inputFile;

        std::vector<std::vector<RGBApixel*>> bluredImage = CreateBluredImage(inputFile,
            parsedArgs.threadsCount,
            parsedArgs.threadPriorities,
            parsedArgs.inputFilePath,
            parsedArgs.coresCount,
            start
        );
        CreateOutputFile(inputFile, parsedArgs.outputFilePath, bluredImage);
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;

        return 1;
    }

    return 0;
}