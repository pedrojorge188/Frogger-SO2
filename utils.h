#ifndef UTILS_H
#define UTILS_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>


#define SERVER_SEMAPHORE TEXT("ServerInstances")
#define SERVER_SHUTDOWN TEXT("ServerExit")
#define SHARED_MEMORY_NAME TEXT("MEM_WITH_DATA")
#define SHARED_MEMORY_SIZE 1024
#endif