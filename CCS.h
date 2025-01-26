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

//Command creation/destruction
CCS_CMD* CCS_CreateCommand();
void CCS_DestroyCommand(CCS_CMD* cmd);

//CMD configurations
bool CCS_SetCmdCommand(CCS_CMD* cmd,char* command);
bool CCS_AddArgument(CCS_CMD* cmd,char* argument);
bool CCS_RemoveArgument(CCS_CMD* cmd,char* argument);
bool CCS_Execute_Command(CCS_CMD* cmd,bool announce); //The bool is use to make the build system output the command

//Helper Functions
char** CCS_GetFilesInDir(CCS_CMD* cmd,int* count,char* path);
bool CCS_DoesFolderExist(CCS_CMD* cmd,char* path);

//Inplementations
#ifdef CCS_IMPL
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
#endif
