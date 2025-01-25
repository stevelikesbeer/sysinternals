#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define LongestStringInFile 514
#define DefaultNumberOfRecords 31428
#define InputFileName "NtApiSymbols.txt"
#define OutputFileName "NtApiSymbolsCSV.csv"

typedef struct 
{
    char Scope[9];
    char Address[18];
    char Name[LongestStringInFile];
} FunctionMetadata;

int ReadDataFromFile(FunctionMetadata **libraryMetaData, char *fileName);
int WriteDataToCSV(FunctionMetadata **libraryMetaData, size_t arrayLength, char *fileName);
char *LTrim(char *stringToTrim, size_t length);

void QuickSort(FunctionMetadata **libraryMetaData, size_t start, size_t end);
size_t Partition(FunctionMetadata **libraryMetaData, size_t start, size_t end);

void BubbleSort( FunctionMetadata **libraryMetaData, size_t arrayLength);
void Swap(FunctionMetadata **first, FunctionMetadata **second);

void *FreeLibraryMetaData(FunctionMetadata **libraryMetaData, size_t numberOfRecords);

int main(int argc, char *argv[])
{
    char *endptr;
    size_t numberOfRecords = argc > 1 ? strtoull(argv[1], &endptr, 0) : DefaultNumberOfRecords;
    if (errno == ERANGE || ( argc > 1 && argv[1][0] == '-') || endptr == argv[1])
    {
        printf("Invalid paramter - Number of records. Using Default Value, %zu\n", DefaultNumberOfRecords);
        numberOfRecords = DefaultNumberOfRecords;
    }

    // Apparently it's a lot safer (and easier for maintenance) to set the size to *variableName rather than Type
    // so that when you change the variable type, you don't have to update the malloc command
    // or count the stars
    FunctionMetadata **libraryMetaData = malloc(numberOfRecords * (sizeof *libraryMetaData)); // too big have to use the heap

    if(!ReadDataFromFile(libraryMetaData, InputFileName))
    {
        puts("Failed to open the input file");
        free(libraryMetaData);
        libraryMetaData = NULL;
        return EXIT_FAILURE;
    } 

    QuickSort(libraryMetaData, 0, numberOfRecords-1); // O(nlogn) average case
    //BubbleSort(libraryMetaData, numberOfRecords-1); // O(n^2)

    if(!WriteDataToCSV(libraryMetaData, numberOfRecords, OutputFileName))
    {
        puts("Failed to open the output file");
        libraryMetaData = FreeLibraryMetaData(libraryMetaData, numberOfRecords);;
        return EXIT_FAILURE;
    }

    libraryMetaData = FreeLibraryMetaData(libraryMetaData, numberOfRecords);
    return EXIT_SUCCESS;
}

int ReadDataFromFile(FunctionMetadata **libraryMetaData, char *fileName)
{
    FILE *inputFile = fopen(fileName, "r");
    if(inputFile == NULL)
        return 0;

    char buffer[LongestStringInFile]; 
    for(size_t i = 0; fgets(buffer, sizeof buffer, inputFile) != NULL; i++)
    {
        FunctionMetadata *metaData = malloc(sizeof *metaData);
        buffer[strcspn(buffer, "\n")] = '\0';
        if(strncmp(buffer, "prv func", 8))
        {
            strcpy(metaData->Scope, "Public");
        }
        else
        {
            strcpy(metaData->Scope, "Private");
        }

        strncpy(metaData->Address, buffer + 11, 17);
        // in strncpy if there are no null terminated character within n characters read, its copy is also not null terminated.
        metaData->Address[17] = '\0'; 

        char *tmp = strchr(buffer, '!'); // returns a pointer to the first occurance of '!'
        if(tmp != NULL)
        {
            tmp++; // remove the !
            tmp = LTrim(tmp, strlen(tmp)); // trim leading spaces;
            strncpy(metaData->Name, tmp, strlen(tmp)); // -1 because we want to get rid of \n
            libraryMetaData[i] = metaData;
        }
    }

    fclose(inputFile);

    return 1;
}

int WriteDataToCSV(FunctionMetadata **libraryMetaData, size_t arrayLength, char *fileName)
{
    FILE *outputFile = fopen(fileName, "w");
    if(outputFile == NULL)
        return 0;

    for(size_t i = 0; i < arrayLength; i++)
    {
        fprintf(outputFile, "%s@%s@%s\n", libraryMetaData[i]->Name, libraryMetaData[i]->Scope, libraryMetaData[i]->Address);
    }

    fclose(outputFile);
    return 1;
}

char *LTrim(char *stringToTrim, size_t length)
{
    if (stringToTrim == NULL)
        return NULL;

    for(int i = 0; i < length; i++)
    {
        if(isspace(stringToTrim[i])) 
            stringToTrim++;
        else
            break;        
    }

    return stringToTrim;
}

void QuickSort(FunctionMetadata **libraryMetaData, size_t start, size_t end)
{
    if(end <= start) return;

    size_t pivot = Partition(libraryMetaData, start, end);
    QuickSort(libraryMetaData, start, pivot-1);
    QuickSort(libraryMetaData, pivot+1, end);
}

size_t Partition(FunctionMetadata **libraryMetaData, size_t start, size_t end)
{
    FunctionMetadata *pivot = libraryMetaData[end];

    size_t i = start - 1;
    for(size_t j = start; j < end; j++)
    {
        if(strcmp(libraryMetaData[j]->Name, pivot->Name) < 0) // if str1 < str2  or if j < pivot
        {
            i++;
            FunctionMetadata *tmp = libraryMetaData[i];
            libraryMetaData[i] = libraryMetaData[j];
            libraryMetaData[j] = tmp;
        }
    }

    i++; // increase i by 1 because thats our new pivot index
    
    // move pivot to the new pivot index
    FunctionMetadata *tmp = libraryMetaData[i];
    libraryMetaData[i] = pivot;
    libraryMetaData[end] = tmp;

    return i;
}

void *FreeLibraryMetaData(FunctionMetadata **libraryMetaData, size_t numberOfRecords)
{
    for (size_t i = 0; i < numberOfRecords; i++)
    {
        free(libraryMetaData[i]);
        libraryMetaData[i] = NULL;
    }

    free(libraryMetaData);

    return NULL;
}

// wow this is so much slower than quicksort, by orders of magnitude. It takes like 20 seconds to run this and only 1 for quicksort.
void BubbleSort(FunctionMetadata **libraryMetaData, size_t arrayLength)
{
    for(size_t i = 0; i < arrayLength; i++)
    {
        for(size_t ii = 0; ii < arrayLength; ii++)
        {
            if(strcmp(libraryMetaData[ii]->Name, libraryMetaData[ii+1]->Name) > 0)
            {
                // im basically passing two pointers to specific indices of the array
                //      into the function, they behave like two brand new arrays (of the same type), 
                //      where I just have to swap the first element of each.
                // for example, pass a pointer that points at 4242, and a pointer that points at 5252
                Swap(&libraryMetaData[ii], &libraryMetaData[ii+1]);  
                /*
                &libraryMetaData == 2323

                &libraryMetaData[2] == 4242

                *libraryMetaData == 12345
                libraryMetaData[0] == 12345

                2323 :contains: 12345
                3232 :contains: 12356
                4242 :contains: 12367       // I dont need to swap anything down below (the actual struct), 
                5252 :contains: 12378       // I just need to swap what item they're pointing at. so 4242 = 12378, and 5252=12367
                6262 :contains: 12389

                    12345 :contains: FunctionMetaData
                    12356 :contains: FunctionMetaData
                    12367 :contains: FunctionMetaData
                    12378 :contains: FunctionMetaData
                    12389 :contains: FunctionMetaData
                */
            }
        }

    }
}

// so these are two pointers to two different array locations
//      they behave like two brand new arrays where I just have to compare the first element of each.
void Swap(FunctionMetadata **first, FunctionMetadata **second) 
{
     // so *first because first is the memory location (4242). 
     //I derefernce it once to get to the value (which holds the memory locations I want to swap)
     //         I want to swap the value at 4242 with the value at 5252
     //         So after the swap 4242 would hold 12378
     //             and 5252 would hold 12367
     //         just as an example
    FunctionMetadata *tmp = *first;    
    *first = *second;
    *second = tmp;
}