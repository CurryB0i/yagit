#pragma once

#ifdef _WIN32
    #include <direct.h>
    #include <windows.h>
    #define MKDIR(path,mode) _mkdir(path)
    #define GETCWD(path,size) _getcwd(path,size)
#else
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
    #define MKDIR(path,mode) mkdir(path,mode)
    #define GETCWD(path,size) getcwd(path,size)
#endif
