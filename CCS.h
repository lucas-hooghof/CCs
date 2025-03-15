#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>


//Basic Commmand structure
typedef struct
{
    char* command;
    char** args;
    int* arglength;
    size_t cmd_size;        // In bytes
    size_t arg_count;
}CCS_CMD;

#ifdef LINUX 
#include <dirent.h>
#include <sys/stat.h>
#endif


//Function declarations


/// @brief CCS_CreateCommand used to make a command pointer-structure
CCS_CMD* CCS_CreateCommand();

/// @brief CCS_DestroyCommand used to destroy a command pointer-structure
/// @param cmd Pointer to a command structure
void CCS_DestroyCommand(CCS_CMD* cmd);

/// @brief CCS_SetCmdCommand used to set the command of cmd to be used in the "CCS_Execute_Command" function
/// @param cmd Pointer to a command structure
/// @param command The name of the command
/// @return Returns if the function was executed correctly
bool CCS_SetCmdCommand(CCS_CMD* cmd,char* command);

/// @brief CCS_AddArgument adds a argument to the command structure to be used in the "CCS_Execute_Command" function
/// @param cmd Pointer to a command structure
/// @param argument A Argument to be added to the argument list
/// @return Returns if the function was executed correctly
bool CCS_AddArgument(CCS_CMD* cmd,char* argument);

/// @brief CCS_RemoveArgument is used to remove a argument from the argument list
/// @param cmd Pointer to a command structure
/// @param argument A Argument to remove from  argument list
/// @return Returns if the function was executed correctly
bool CCS_RemoveArgument(CCS_CMD* cmd,char* argument);

/// @brief CCS_Execute_Command is used to execute a command 
/// @param cmd Pointer to a command structure
/// @param announce Make the command get outputted to the console
/// @return Returns if the function was executed correctly
bool CCS_Execute_Command(CCS_CMD* cmd,bool announce); 

/// @brief CCS_GetFilesInDir is used to get all the files in a directory with depth
/// @param cmd Pointer to a command structure
/// @param count A interger that gets filled with the amount of found files
/// @param path The path to the files needed to be found
/// @return Returns if the function was executed correctly
char** CCS_GetFilesInDir(CCS_CMD* cmd,int* count,char* path);

/// @brief CCS_DoesFolderExist is used to check if a folder exists
/// @param cmd Pointer to a command structure
/// @param path The path to the directory
/// @return Returns if the function was executed correctly
bool CCS_DoesFolderExist(CCS_CMD* cmd,char* path);

/// @brief CCS_CreateDirectory is used to create a directory
/// @param directory The directory to be created
/// @param announce If the command should be outputted to the Console
/// @attention CCS_CreateDirectory cant make directories with depth use "CCS_CreateDirectoryDepth" for that
void CCS_CreateDirectory(char* directory,bool announce);

/// @brief CCS_CreateDirectoryDepth is used to create a directory with depth like "/dir/dir/dir"
/// @param directory The directory to be created
/// @param announce  If the command should be outputted to the Console
void CCS_CreateDirectoryDepth(char* directory,bool announce);

/// @brief CCS_RemoveDirectory is used to remove a directory
/// @param directory The directory to be removed
/// @param announce If the command should be outputted to the Console
/// @attention This can be used with depth
void CCS_RemoveDirectory(char* directory,bool announce);

/// @brief CCS_SetCurrent is used to set Build System Vars
/// @param Which The string to change
/// @param to The string to change it to
void CCS_SetCurrent(char** Which,char* to);

/// @brief CCS_AssembleFile is a helper function used to assemble a file into a object file
/// @param file The input asm file
/// @param flags The flags needed for it
/// @param output The output place and name
/// @param announce If set the command will be send out to the console
/// @attention !!! Dont add "-o" to the flags !!!
void CCS_AssembleFile(char* file,char* flags,char* output,bool announce);

/// @brief CCS_LinkFile is a helper function used to link a file
/// @param file The input object file
/// @param flags The flags needed for it
/// @param output The output place and name
/// @param announce If set the command will be send out to the console
/// @attention This function takes in 1 file and !!! Dont add "-o" to the flags !!!
void CCS_LinkFile(char* file,char* flags,char* output,bool announce);

/// @brief CCS_WriteDataToFile is a helper function used to Write Data to a file like a MBR
/// @param Data The pointer to the data
/// @param size The size of the data in bytes
/// @param offset The offset into the file in bytes
/// @param file The path to the file
/// @param announce If set the command will be send out to the console
void CCS_WriteDataToFile(void* Data,size_t size,size_t offset,char* file,bool announce);

/// @brief CCS_CreateFile is a helper function used to create empty files
/// @param name This is the path to where and what it should be called
/// @param size This is the wanted size of the file
void CCS_CreateFile(char* name,size_t size);

extern char* Current_Ccompiler;
extern char* Current_CPPcompiler;
extern char* Current_Linker;
extern char* Current_Assembler;

//Inplementations
#ifdef CCS_IMPL
    char* Current_Ccompiler = NULL;
    char* Current_CPPcompiler = NULL;
    char* Current_Linker = NULL;
    char* Current_Assembler = NULL;
    CCS_CMD* CCS_CreateCommand()
    {
        CCS_CMD* cmd = (CCS_CMD*)malloc(sizeof(CCS_CMD));
        if (cmd == NULL)
        {
            fprintf(stderr,"Cant create Command!\n");
            return NULL;
        }
        memset(cmd,0,sizeof(CCS_CMD));
        return cmd;
    }

    void CCS_DestroyCommand(CCS_CMD* cmd)
    {
        if (cmd->command != NULL) {free(cmd->command);}
        if (cmd->args != NULL)
        {
            for (int i = 0; i < cmd->arg_count; i++)
            {
                free(cmd->args[i]);
            }
            free(cmd->arglength);

            free(cmd->args);
        }
        free(cmd);
    }

    bool CCS_SetCmdCommand(CCS_CMD* cmd,char* command)
    {
        size_t commandlen = strlen(command);
        cmd->command = (char*)malloc(commandlen+1);
        if (cmd->command == NULL)
        {
            fprintf(stderr,"Failed to set command\n");
            return false;
        }
        memset(cmd->command,0,commandlen+1);

        strncpy(cmd->command,command,commandlen);
        cmd->cmd_size = commandlen;
        return true;
    }

    bool CCS_AddArgument(CCS_CMD* cmd,char* argument)
    {
        if (cmd->args == NULL && cmd->arg_count == 0) 
        {
            cmd->args = (char**)malloc(sizeof(char*));
            if (cmd->args == NULL)
            {
                fprintf(stderr,"Failed to allocate memory for argument");
                return false;
            }
        }
        else 
        {
            cmd->args = (char**)realloc(cmd->args,cmd->arg_count * sizeof(char*) + sizeof(char*));
            if (cmd->args == NULL)
            {
                fprintf(stderr,"Failed to reallocate memory for the argument");
                return false;
            }
        }

        size_t argumentLength = strlen(argument);

        cmd->args[cmd->arg_count++] = (char*)malloc(argumentLength+1);
        if (cmd->args[cmd->arg_count-1] == NULL)
        {
            fprintf(stderr,"Failed to allocate memory for the argument string");
            return false;
        }

        cmd->arglength = (int*)realloc(cmd->arglength,cmd->arg_count * sizeof(int));
        if (cmd->arglength == NULL)
        {
            fprintf(stderr,"Failed to increment the arglength array");
            return false;
        }
        cmd->arglength[cmd->arg_count-1] = argumentLength;

        strncpy(cmd->args[cmd->arg_count-1],argument,argumentLength);

        return true;
    }

    bool CCS_Execute_Command(CCS_CMD* cmd,bool announce)
    {
        size_t commandlength = cmd->cmd_size;
        size_t i = 0;

        for (int argi = 0; argi < cmd->arg_count; argi++)
        {
            commandlength+=cmd->arglength[argi];
        }
        commandlength += cmd->arg_count;

        char* commandbuffer = (char*)malloc(commandlength+1);

        if (commandbuffer == NULL)
        {
            fprintf(stderr,"Failed to allocate memory for the command buffer!");
            return false;
        }

        strncpy(commandbuffer,cmd->command,cmd->cmd_size);
        i+=cmd->cmd_size;
        
        for (size_t argi = 0; argi < cmd->arg_count; argi++)
        {
            commandbuffer[i] = ' ';
            i++;
            memcpy(commandbuffer+i,cmd->args[argi],cmd->arglength[argi]);
            i+=cmd->arglength[argi];    
        }
        commandbuffer[i] = '\0';
        if (announce)
        {
            printf("\x1b[1m>> Executing: %s\x1b[1;39m\n",commandbuffer);
        }

        system(commandbuffer);

        free(commandbuffer);

        return true;
    }

    bool CCS_RemoveArgument(CCS_CMD* cmd,char* argument)
    {
        for (size_t argi = 0; argi < cmd->arg_count; argi++)
        {
            if (!strcmp(cmd->args[argi],argument))
            {
                free(cmd->args[argi]);

                //Reorder the arrays
                for (size_t j = argi; j < cmd->arg_count -1; j++)
                {
                    cmd->args[j] = cmd->args[j+1];
                    cmd->arglength[j] = cmd->arglength[j+1];
                }

                cmd->arg_count--;
                
            }
        }
    }
    #ifdef LINUX
    bool CCS_DoesFolderExist(CCS_CMD* cmd,char* path)
    {
        DIR* dir = opendir(path);
        if (dir == NULL)
        {
            return false;
        }
        closedir(dir);
        return true;
    }
    char** CCS_GetFilesInDir(CCS_CMD* cmd,int* count,char* path)
    {
        DIR* dir = opendir(path);
        if (dir == NULL)
        {
            printf("Failed to open a directory\n");
            return NULL;
        }
        struct dirent* direntry;
        char** files = NULL;
        while ((direntry = readdir(dir)) != NULL)
        {
            
            if (!strcmp(direntry->d_name,"..") || !strcmp(direntry->d_name,"."))
            {
                continue;
            }
            if (direntry->d_type == DT_DIR)
            {
                int newcount = 0;
                char** inner = CCS_GetFilesInDir(cmd,&newcount,direntry->d_name);
                files = (char**) realloc(files,*count + newcount * sizeof(char*));
                for (size_t innercount = *count; innercount < newcount + *count; innercount++)
                {
                    files[innercount] = (char*)malloc(strlen(inner[innercount - *count]) + 1);
                    strcpy(files[innercount],inner[innercount - *count]);
                }
                *count += newcount;
            }
            else if (direntry->d_type == DT_REG)
            {
                if (*count == 0)
                {
                    files = (char**)malloc(sizeof(char*));
                }
                else {
                    files = (char**)realloc(files,(*count+1) * sizeof(char*));
                }
                files[*count] = (char*)malloc(strlen(direntry->d_name)+1+strlen(path)+1);
                strcpy(files[*count],path);
                strncat(files[*count],"/",1);
                strcat(files[*count],direntry->d_name);
                *count += 1;
            }
        }

        closedir(dir);

        return files;
    }   

    #endif
    void CCS_CreateDirectory(char* directory,bool announce)
    {   
        CCS_CMD* cmd = CCS_CreateCommand();
        CCS_SetCmdCommand(cmd,"mkdir");
        CCS_AddArgument(cmd,directory);

        CCS_Execute_Command(cmd,announce);
        CCS_DestroyCommand(cmd);
    }

    void CCS_CreateDirectoryDepth(char* directory,bool announce)
    {
        CCS_CMD* cmd = CCS_CreateCommand();
        CCS_SetCmdCommand(cmd,"mkdir");
        CCS_AddArgument(cmd,"-p");
        CCS_AddArgument(cmd,directory);

        CCS_Execute_Command(cmd,announce);
        CCS_DestroyCommand(cmd);
    }
    void CCS_RemoveDirectory(char* directory,bool announce)
    {   
        CCS_CMD* cmd = CCS_CreateCommand();
        CCS_SetCmdCommand(cmd,"rm");
        CCS_AddArgument(cmd,"-rf");
        CCS_AddArgument(cmd,directory);

        CCS_Execute_Command(cmd,announce);
        CCS_DestroyCommand(cmd);
    }
    void CCS_SetCurrent(char** Which,char* to)
    {
        if (*Which != NULL) {free(*Which);}
        *Which = (char*)malloc(strlen(to)+1);
        strncpy(*Which,to,strlen(to)+1);
    }

    void CCS_AssembleFile(char* file,char* flags,char* output,bool announce)
    {
        CCS_CMD* command = CCS_CreateCommand();
        CCS_SetCmdCommand(command,Current_Assembler);
        CCS_AddArgument(command,file);
        CCS_AddArgument(command,flags);
        CCS_AddArgument(command,"-o");
        CCS_AddArgument(command,output);

        CCS_Execute_Command(command,announce);
        CCS_DestroyCommand(command);
    }

    void CCS_LinkFile(char* file,char* flags,char* output,bool announce)
    {
        CCS_CMD* command = CCS_CreateCommand();
        CCS_SetCmdCommand(command,Current_Linker);
        CCS_AddArgument(command,file);
        CCS_AddArgument(command,flags);
        CCS_AddArgument(command,"-o");
        CCS_AddArgument(command,output);

        CCS_Execute_Command(command,announce);
        CCS_DestroyCommand(command);
    }

    void CCS_WriteDataToFile(void* Data,size_t size,size_t offset,char* file,bool announce)
    {
        if (announce == true)
        {
            printf("Writing Data to %s \n",file);
        }

        FILE* filep = fopen(file,"wb");
        fseek(filep,offset,SEEK_SET);
        fwrite(Data,1,size,filep);

        fclose(filep);
    }

    void CCS_CreateFile(char* name,size_t size)
    {
        size_t cmdlength = strlen("dd if=/dev/zero ") + strlen("of=") + strlen(name) + strlen(" bs=1 count=") + 20;
        char* stringcmd = (char*)malloc(cmdlength);
        if (!stringcmd) {printf("Cant create file!\n"); exit(EXIT_FAILURE);}
        snprintf(stringcmd,cmdlength,"dd if=/dev/zero of=%s bs=1 count=%d",name,size);

        system(stringcmd);

        free(stringcmd);
    }
#endif

