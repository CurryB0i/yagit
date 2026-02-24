#pragma once

#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
    #include <direct.h>
    #include <winsock2.h>
    #include <windows.h>
    #define MKDIR(path, mode) _mkdir(path)
    #define GETCWD(path, size) _getcwd(path, size)
    #define RMDIR(path) _rmdir(path)
    #define STAT(path, stptr) stat(path, stptr)
    #define SLEEP(ms) Sleep(ms)
    #define GET_ABS_PATH(abs_path, rel_path, max_len) _fullpath(abs_path, rel_path, max_len)
    #define PATH_SEP '\\'
    #define ST_MTIM_SEC(st) st.st_mtime
    #define ST_MTIM_NSEC(st) 0
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #define MKDIR(path,mode) mkdir(path,mode)
    #define GETCWD(path,size) getcwd(path,size)
    #define RMDIR(path) rmdir(path)
    #define STAT(path, stptr) lstat(path, stptr)
    #define SLEEP(ms) sleep((unsigned int)(ms/1000))
    #define GET_ABS_PATH(abs_path, rel_path, max_len) realpath(rel_path, abs_path)
    #define PATH_SEP '/'
    #define ST_MTIM_SEC(st) st.st_mtim.tv_sec
    #define ST_MTIM_NSEC(st) st.st_mtim.tv_nsec
#endif
