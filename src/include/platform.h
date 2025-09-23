#pragma once

#ifdef _WIN32
    #include <direct.h>
    #include <winsock2.h>
    #include <windows.h>
    #define MKDIR(path,mode) _mkdir(path)
    #define GETCWD(path,size) _getcwd(path,size)
    #define PATH_SEP '\\'
    #define ST_MTIM_SEC(st) st.st_mtime
    #define ST_MTIM_NSEC(st) 0
#else
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #define MKDIR(path,mode) mkdir(path,mode)
    #define GETCWD(path,size) getcwd(path,size)
    #define PATH_SEP '/'
    #define ST_MTIM_SEC(st) st.st_mtim.tv_sec
    #define ST_MTIM_NSEC(st) st.st_mtim.tv_nsec
#endif
