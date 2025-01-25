#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define LongestStringInFile 514
#define DefaultNumberOfRecords 31428
#define InputFileName "NtApiSymbolsCSV.csv"

typedef struct 
{
    char Scope[9];
    char Address[18];
    char Name[LongestStringInFile];
} FunctionMetadata;

int ReadInputData(FunctionMetadata **libraryMetaData, char *fileLocation);
void PrintOptions();
void GetUserCommand(char *userCommand, size_t *userCommandLength);

void PrintLibraryFunctionsWithPrefix(FunctionMetadata **libraryMetaData, char userCommand[30], size_t userCommandLength, size_t numberOfRecords);

int main(int argc, char *argv[])
{
    char *endptr;
    size_t numberOfRecords = argc > 1 ? strtoull(argv[1], &endptr, 0) : DefaultNumberOfRecords;
    if (errno == ERANGE || ( argc > 1 && argv[1][0] == '-') || endptr == argv[1])
    {
        printf("Invalid paramter - Number of records. Using Default Value, %zu\n", DefaultNumberOfRecords);
        numberOfRecords = DefaultNumberOfRecords;
    }

    FunctionMetadata **libraryMetaData = malloc(sizeof(FunctionMetadata) * numberOfRecords);

    if(!ReadInputData(libraryMetaData, InputFileName))
    {
        puts("Failed to open the input file");
        free(libraryMetaData);
        libraryMetaData = NULL;
        return EXIT_FAILURE;
    }

    char userCommand[30];
    size_t userCommandLength;
    while(strcmp(userCommand, "quit") != 0 && strcmp(userCommand, "exit") != 0)
    {
        PrintOptions();
        GetUserCommand(userCommand, &userCommandLength);

        if(strcmp(userCommand, "quit") == 0 || strcmp(userCommand, "exit") == 0)
        {
            continue;
        }
        else if (strcmp(userCommand, "1") == 0)
        {
            for(size_t i = 0; i < numberOfRecords; i++)
            {
                printf("%s, %s, %s\n", libraryMetaData[i]->Name, libraryMetaData[i]->Scope, libraryMetaData[i]->Address);
            }
        }
        else if (strcmp(userCommand, "2") == 0)
        {
            strcpy(userCommand, "Inbv");
            PrintLibraryFunctionsWithPrefix(libraryMetaData, userCommand, strlen(userCommand), numberOfRecords);
        }
        else if (strcmp(userCommand, "3") == 0)
        {
            strcpy(userCommand, "Iop");
            PrintLibraryFunctionsWithPrefix(libraryMetaData, userCommand, strlen(userCommand), numberOfRecords);
        }
        else // specific prefix requested, entered in userCommand
        {
            if(userCommandLength == 0) 
            {
                puts("Invalid entry. A Command must be entered. Press enter to continue or type `exit` to quit");
                GetUserCommand(userCommand, &userCommandLength);
                continue;
            }

            PrintLibraryFunctionsWithPrefix(libraryMetaData, userCommand, userCommandLength, numberOfRecords);
        }
        puts("\nPress enter to continue or type `exit` to quit");
        GetUserCommand(userCommand, &userCommandLength);
    }

    for(size_t i = 0; i < numberOfRecords; i++)
    {
        free(libraryMetaData[i]);
        libraryMetaData[i] = NULL;
    }
    free(libraryMetaData);

    return EXIT_SUCCESS;
}

void PrintLibraryFunctionsWithPrefix(FunctionMetadata **libraryMetaData, char userCommand[30], size_t userCommandLength, size_t numberOfRecords)
{
    bool found = false;
    for (size_t i = 0; i < numberOfRecords; i++)
    {
        if (strncmp(userCommand, libraryMetaData[i]->Name, userCommandLength) == 0)
        {
            if ((strcmp(userCommand, "Io") == 0) && libraryMetaData[i]->Name[2] == 'p') // make sure we don't include Iop commands when searching for Io commands
            {
                continue;
            }

            printf("%s, %s, %s\n", libraryMetaData[i]->Name, libraryMetaData[i]->Scope, libraryMetaData[i]->Address);
            found = true;
        }
    }

    if (!found)
    {
        puts("No records found");
    }
}

int ReadInputData(FunctionMetadata **libraryMetaData, char *fileName)
{
    FILE *inputFile = fopen(fileName, "r");
    if(inputFile == NULL)
        return 0;

    char buffer[LongestStringInFile];
    size_t attributeLength = 0;
    for(size_t i = 0; fgets(buffer, LongestStringInFile, inputFile); i++)
    {
        FunctionMetadata *metaData = malloc(sizeof(FunctionMetadata));
        buffer[strcspn(buffer, "\n")] = '\0';
        char *name = strtok(buffer, "@");
        if(name == NULL)
        {
            printf("Invalid Input Data. Name missing on line %zu \n", i);
            fclose(inputFile);
            return 0;
        }
        strcpy(metaData->Name,name);

        char *scope = strtok(NULL, "@");
        if(scope == NULL)
        {
            printf("Invalid Input Data. Scope missing on line %zu \n", i);
            fclose(inputFile);
            return 0;
        }
        strcpy(metaData->Scope, scope);

        char *address = strtok(NULL, "@");
        if(address == NULL)
        {
            printf("Invalid Input Data. Address missing on line %zu \n", i);
            fclose(inputFile);
            return 0;
        }
        strcpy(metaData->Address,address);

        libraryMetaData[i] = metaData;
    }

    fclose(inputFile);

    return 1;
}
void PrintOptions()
{
    puts("Type a command (or search string):");
    puts("-------- General Categories --------");
    puts("    1    - Print All Names");
    puts("    2    - (Inbv) Boot Video Driver - Called from Kernal Mode, Not Documented or Exported");
    puts("    3    - (Iop) Internal I/O Manager Support Functions - Defined Global Symbols but not exported");
    puts("-------- Specific prefixes --------");
    puts("    Alpc  - Advanced Local Procedure Calls");
    puts("    Cc    - Common Cache");
    puts("    Cm    - Configuration Manager");
    puts("    Dbg   - Kernal Debug Support");
    puts("    Dbgk  - Debugging Framework for USER mode");
    puts("    Em    - Errata Manager");
    puts("    Etw   - Event Tracing for Windows");
    puts("    Ex    - Executive Support Routines");
    puts("    FsRtl - File System Runtime Library");
    puts("    Hv    - Hive Library");
    puts("    Hvl   - Hypervisor Library");
    puts("    Io    - I/O Manager");
    puts("    Kd    - Kernal Debugger");
    puts("    Ke    - Kernal");
    puts("    Kse   - Kernal Shim Engine");
    puts("    Lsa   - Local Security Authority");
    puts("    Mm    - Memory Manager");
    puts("    Nt    - NT System Services (Available through USER mode through system calls)");
    puts("    Ob    - Object Manager");
    puts("    Pf    - Prefetcher");
    puts("    Po    - Power Manager");
    puts("    PoFx  - Power Framework");
    puts("    Pp    - PnP Manager");
    puts("    Ppm   - Processor Power Manager");
    puts("    Ps    - Process Support");
    puts("    Rtl   - Runtime Library");
    puts("    Se    - Security Reference Monitor");
    puts("    Sm    - Store Manager");
    puts("    Tm    - Transaction Manager");
    puts("    Ttm   - Terminal Timeout Manager");
    puts("    Vf    - Driver Verifier");
    puts("    Vsl   - Virtual Secure Mode Library");
    puts("    Wdi   - Windows Diagnostic Infrastructure");
    puts("    Wpf   - Windows  Finger Print");
    puts("    Whea  - Windows Hardware Error Architecture");
    puts("    Wmi   - Windows Managment Instrumentation");
    puts("    Zw    - Mirror entry point for system services (beginning with Nt) that sets previous  access mode to kernel\n");
    puts("    `exit` to quit");
}

void GetUserCommand(char *userCommand, size_t *userCommandLength)
{
    printf("> ");
    fgets(userCommand, 30, stdin);
    *userCommandLength = strlen(userCommand)-1;
    userCommand[*userCommandLength] = '\0';

}