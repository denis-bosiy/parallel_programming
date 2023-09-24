#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include "EasyBmp.h"
#include <vector>
#include <chrono>

#define MAX_THREADS_COUNT 16
#define MAX_KERNELS_COUNT 4

struct Args
{
    char* inputFilePath;
    char* outputFilePath;
    int kernelsCount;
    int threadsCount;

    Args(char* _inputFilePath, char* _outputFilePath, int _kernelsCount, int _threadsCount) {
        inputFilePath = _inputFilePath;
        outputFilePath = _outputFilePath;
        kernelsCount = _kernelsCount;
        threadsCount = _threadsCount;
    }
};

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
    // Gaussian blur algorithm
    const std::vector<RGBApixel*> imageRow = *(std::vector<RGBApixel*>*)(lpParam);
    std::cout << imageRow.size() << std::endl;
    //for (int i = 0; i < imageRow.size(); i++)
    //{
    //    RGBApixel* sumPixel = new RGBApixel();
    //    if (i == 0 && imageRow.size() > 1)
    //    {
    //        sumPixel->Red = (2 * imageRow[i]->Red) % 255;
    //        sumPixel->Green = (2 * imageRow[i]->Green) % 255;
    //        sumPixel->Blue = (2 * imageRow[i]->Blue) % 255;
    //        sumPixel->Alpha = (2 * imageRow[i]->Alpha) % 255;

    //        sumPixel->Red = sumPixel->Red + imageRow[i + 1]->Red % 255;
    //        sumPixel->Green = sumPixel->Green + imageRow[i + 1]->Green % 255;
    //        sumPixel->Blue = sumPixel->Blue + imageRow[i + 1]->Blue % 255;
    //        sumPixel->Alpha = sumPixel->Alpha + imageRow[i + 1]->Alpha % 255;

    //        sumPixel->Red = sumPixel->Red / 3;
    //        sumPixel->Green = sumPixel->Green / 3;
    //        sumPixel->Blue = sumPixel->Blue / 3;
    //        sumPixel->Alpha = sumPixel->Alpha / 3;
    //    }
    //    else if (i == imageRow.size() - 1 && imageRow.size() > 1)
    //    {
    //        sumPixel->Red = (2 * imageRow[i]->Red) % 255;
    //        sumPixel->Green = (2 * imageRow[i]->Green) % 255;
    //        sumPixel->Blue = (2 * imageRow[i]->Blue) % 255;
    //        sumPixel->Alpha = (2 * imageRow[i]->Alpha) % 255;

    //        sumPixel->Red = sumPixel->Red + imageRow[i - 1]->Red % 255;
    //        sumPixel->Green = sumPixel->Green + imageRow[i - 1]->Green % 255;
    //        sumPixel->Blue = sumPixel->Blue + imageRow[i - 1]->Blue % 255;
    //        sumPixel->Alpha = sumPixel->Alpha + imageRow[i - 1]->Alpha % 255;

    //        sumPixel->Red = sumPixel->Red / 3;
    //        sumPixel->Green = sumPixel->Green / 3;
    //        sumPixel->Blue = sumPixel->Blue / 3;
    //        sumPixel->Alpha = sumPixel->Alpha / 3;
    //    }
    //    else if (imageRow.size() > 2) {
    //        sumPixel->Red = imageRow[i - 1]->Red % 255;
    //        sumPixel->Green = imageRow[i - 1]->Green % 255;
    //        sumPixel->Blue = imageRow[i - 1]->Blue % 255;
    //        sumPixel->Alpha = imageRow[i - 1]->Alpha % 255;

    //        sumPixel->Red = sumPixel->Red + (2 * imageRow[i]->Red) % 255;
    //        sumPixel->Green = sumPixel->Green + (2 * imageRow[i]->Green) % 255;
    //        sumPixel->Blue = sumPixel->Blue + (2 * imageRow[i]->Blue) % 255;
    //        sumPixel->Alpha = sumPixel->Alpha + (2 * imageRow[i]->Alpha) % 255;

    //        sumPixel->Red = sumPixel->Red + imageRow[i + 1]->Red % 255;
    //        sumPixel->Green = sumPixel->Green + imageRow[i + 1]->Green % 255;
    //        sumPixel->Blue = sumPixel->Blue + imageRow[i + 1]->Blue % 255;
    //        sumPixel->Alpha = sumPixel->Alpha + imageRow[i + 1]->Alpha % 255;

    //        sumPixel->Red = sumPixel->Red / 4;
    //        sumPixel->Green = sumPixel->Green / 4;
    //        sumPixel->Blue = sumPixel->Blue / 4;
    //        sumPixel->Alpha = sumPixel->Alpha / 4;
    //    }
    //    else {
    //        continue;
    //    }
    //    std::cout << (int)sumPixel->Red << std::endl;
    //    imageRow[i]->Red = sumPixel->Red;
    //    imageRow[i]->Green = sumPixel->Green;
    //    imageRow[i]->Blue = sumPixel->Blue;
    //    imageRow[i]->Alpha = sumPixel->Alpha;
    //}

    ExitThread(0);
}

Args ProcessArgs(int argc, char* argv[])
{
    if (argc - 1 != 4) {
        throw std::length_error("There should be 4 arguments: inputImagePath, outputImagePath, kernelsCount and threadsCount");
    }

    try {
        std::ifstream testStream;
        testStream.open(argv[1]);
    }
    catch (std::exception) {
        throw std::invalid_argument("Can not open input image");
    }

    const int threadsCount = std::stoi(argv[3]);
    if (threadsCount < 1 || threadsCount > MAX_THREADS_COUNT) {
        const std::string errorMsg = "Threads count should be less than " +
            std::to_string(MAX_THREADS_COUNT + 1) +
            " and more than 0";
        throw std::invalid_argument(errorMsg);
    }

    const int kernelsCount = std::stoi(argv[4]);
    if (kernelsCount < 1 || kernelsCount > MAX_KERNELS_COUNT) {
        const std::string errorMsg = "Kernels count should be less than " +
            std::to_string(MAX_KERNELS_COUNT + 1) +
            " and more than 0";
        throw std::invalid_argument(errorMsg);
    }

    return * new Args(argv[1], argv[2], threadsCount, kernelsCount);
}

void ClearThreads(HANDLE* handles, int threadsCount) {
    for (int i = 0; i < threadsCount; i++) {
        //delete handles[i];
    }
}

void ProcessRows(int kernelsCount, HANDLE* handles, const std::vector<std::vector<RGBApixel*>> imageRows) {
    for (int i = 0; i < imageRows.size(); i++)
    {
        std::vector<RGBApixel*> imageRow = {};
        for (int j = 0; j < imageRows[i].size(); j++)
        {
            RGBApixel* newPixel = new RGBApixel();
            newPixel->Red = imageRows[i][j]->Red;
            newPixel->Green = imageRows[i][j]->Green;
            newPixel->Blue = imageRows[i][j]->Blue;
            newPixel->Alpha = imageRows[i][j]->Alpha;

            imageRow.push_back(newPixel);
        }
        handles[i] = CreateThread(NULL, 0, &ThreadProc, &imageRow, CREATE_SUSPENDED, NULL);
        SetProcessAffinityMask(handles[i], kernelsCount);
    }

    for (int i = 0; i < imageRows.size(); i++) {
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

int main(int argc, char* argv[])
{
    try
    {
        auto start = std::chrono::high_resolution_clock::now();
        Args parsedArgs = ProcessArgs(argc, argv);
        HANDLE* handles = new HANDLE[parsedArgs.threadsCount];
        BMP inputFile;
        inputFile.ReadFromFile(parsedArgs.inputFilePath);
        std::vector<std::vector<RGBApixel*>> totalImageRows = {};

        for (int rowIndex = 0; rowIndex < inputFile.TellHeight(); rowIndex = rowIndex + parsedArgs.threadsCount)
        {
            std::vector<std::vector<RGBApixel*>> imageRows = {};
            for (int i = 0; i < parsedArgs.threadsCount; i++)
            {
                imageRows.push_back(ReadRowFromFile(inputFile, rowIndex + i));
            }

            ProcessRows(parsedArgs.kernelsCount, handles, imageRows);

            for (int i = 0; i < parsedArgs.threadsCount; i++)
            {
                totalImageRows.push_back(imageRows[i]);
            }
        }
        for (int rowIndex = 0; rowIndex < inputFile.TellHeight(); rowIndex++)
        {
            for (int columnIndex = 0; columnIndex < inputFile.TellWidth(); columnIndex++)
            {
                //std::cout << (int)totalImageRows[rowIndex][columnIndex]->Red << std::endl;
                inputFile(columnIndex, rowIndex)->Red = totalImageRows[rowIndex][columnIndex]->Red;
                inputFile(columnIndex, rowIndex)->Green = totalImageRows[rowIndex][columnIndex]->Green;
                inputFile(columnIndex, rowIndex)->Blue = totalImageRows[rowIndex][columnIndex]->Blue;
                inputFile(columnIndex, rowIndex)->Alpha = totalImageRows[rowIndex][columnIndex]->Alpha;
            }
        }
        inputFile.WriteToFile(parsedArgs.outputFilePath);

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