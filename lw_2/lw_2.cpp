#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include "EasyBmp.h"
#include <vector>
#include <chrono>

#define MAX_THREADS_COUNT 16
#define MAX_KERNELS_COUNT 4
#define M_PI 3.14

struct Args
{
    char* inputFilePath;
    char* outputFilePath;
    int kernelsCount; 
    int threadsCount;

    Args(char* _inputFilePath, char* _outputFilePath, int _kernelsCount, int _threadsCount)
    {
        inputFilePath = _inputFilePath;
        outputFilePath = _outputFilePath;
        kernelsCount = _kernelsCount;
        threadsCount = _threadsCount;
    }
};

double GetGaussianFunctionValue(int x)
{
    const double SIGMA_VALUE = 10;

    return (1 / (sqrt(M_PI * SIGMA_VALUE * SIGMA_VALUE))) * exp(-(x*x) / (2 * SIGMA_VALUE * SIGMA_VALUE));
}

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
    const std::vector<RGBApixel*> imageRow = *(std::vector<RGBApixel*>*)(lpParam);

    // Gaussian blur algorithm
    for (int i = 0; i < imageRow.size(); i++)
    {
        RGBApixel* sumPixel = new RGBApixel();
        sumPixel->Red = 0;
        sumPixel->Green = 0;
        sumPixel->Blue = 0;
        sumPixel->Alpha = 0;

        double gaussianFunctionValuesSum = 0;
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

    ExitThread(0);
}

Args ProcessArgs(int argc, char* argv[])
{
    if (argc - 1 != 4)
    {
        throw std::length_error("There should be 4 arguments: inputImagePath, outputImagePath, kernelsCount and threadsCount");
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

    const int kernelsCount = std::stoi(argv[4]);
    if (kernelsCount < 1 || kernelsCount > MAX_KERNELS_COUNT)
    {
        const std::string errorMsg = "Kernels count should be less than " +
            std::to_string(MAX_KERNELS_COUNT + 1) +
            " and more than 0";
        throw std::invalid_argument(errorMsg);
    }

    return * new Args(argv[1], argv[2], threadsCount, kernelsCount);
}

void ClearThreads(HANDLE* handles, int threadsCount)
{
    for (int i = 0; i < threadsCount; i++)
    {
        CloseHandle(handles[i]);
    }
}

void ProcessRows(int kernelsCount, HANDLE* handles, const std::vector<std::vector<RGBApixel*>> imageRows)
{
    for (int i = 0; i < imageRows.size(); i++)
    {
        std::vector<RGBApixel*>* imageRow = new std::vector<RGBApixel*>(0);
        for (int j = 0; j < imageRows[i].size(); j++)
        {
            (*imageRow).push_back(imageRows[i][j]);
        }
        handles[i] = CreateThread(NULL, 0, &ThreadProc, imageRow, CREATE_SUSPENDED, NULL);
        SetProcessAffinityMask(handles[i], kernelsCount);
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
    const char* inputFilePath,
    int kernelsCount
)
{
    std::vector<std::vector<RGBApixel*>> totalImageRows = {};
    HANDLE* handles = new HANDLE[threadsCount];
    inputFile.ReadFromFile(inputFilePath);

    for (int rowIndex = 0; rowIndex < inputFile.TellHeight(); rowIndex = rowIndex + threadsCount)
    {
        std::vector<std::vector<RGBApixel*>> imageRows = {};
        for (int i = 0; i < threadsCount; i++)
        {
            imageRows.push_back(ReadRowFromFile(inputFile, rowIndex + i));
        }

        ProcessRows(kernelsCount, handles, imageRows);

        for (int i = 0; i < threadsCount; i++)
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
            parsedArgs.inputFilePath,
            parsedArgs.kernelsCount
        );
        CreateOutputFile(inputFile, parsedArgs.outputFilePath, bluredImage);

        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << "Execution time: " << std::to_string(duration.count()) << std::endl;
    } catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;

        return 1;
    }

    return 0;
}