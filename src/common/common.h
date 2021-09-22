#pragma once

#include <unistd.h>
#include <sys/resource.h>

enum RUNNING_CONDITION{
    SUCCESS = 0,                //运行成功
    RUNTIME_ERROR,              //运行时错误
    TIME_LIMIT_EXCEED,          //超时
    MEMORY_LIMIT_EXCEED,        //爆内存
    SEGMENTATION_FAULT,         //段错误
    UNKNOWN_ERROR,              //未知错误
    OUTPUT_LIMIT_EXCEED,        //爆输出
    UNABLE_TO_GET_INPUT,        //无法输入
    UNABLE_TO_MAKE_OUTPUT,      //无法输出
    UNABLE_TO_EXECVE,           //无法执行程序 
    UNABLE_TO_SET_UID,          //无法以低权限用户执行
    UNABLE_TO_LIMIT_MEM,        //无法限制内存
    UNABLE_TO_LIMIT_CPU_TIME,   //无法限制cpu时间
    UNABLE_TO_LIMIT_OUTPUT,     //无法限制输出大学
    UNABLE_TO_LIMIT_STACK,      //无法限制堆栈
    UNABLE_TO_SECCOMP,          //无法限制系统调用
    UNABLE_TO_LIMIT_REAL_TIME,  //无法限制真实时间
    CONFIG_ERROR                //设置错误
};

enum LIMIT_DEFAULT{
    CPU_TIME_LIMIT_DEFAULT = 4000,
    REAL_TIME_LIMIE_DEFAULT = 10000,
    MEMORY_LIMIT_DEFAULT = 1024*64,
    WALL_MEMORY_LIMIT_DEFAULT = 1024*64*4,
    OUTPUT_SIZE_LIMIT_DEFAULT = 10000,
    USER_ID_DEFAULT = -1
};

struct ExecveConfig
{
    rlim_t cpuTimeLimit;        //单位ms
    rlim_t realTimeLimit;       //单位ms
    rlim_t memoryLimit;         //单位kB
    rlim_t wallMemoryLimit;     //单位kB
    rlim_t outputSizeLimit;     //单位kB
    uid_t userId;               //以该userid对应用户执行提交程序
    char *execvePath;           //
    char *stdinPath;            //
    char *stdoutPath;           //
    char *stderrPath;           //
};

struct ExecveResult{
    rlim_t cpuTimeCost;                 //单位毫秒
    rlim_t realTimeCost;                //单位毫秒
    rlim_t memoryCost;                  //单位kB
    enum RUNNING_CONDITION condition;   //
};

void initializeConfig(struct ExecveConfig *const config);
void initializeResult(struct ExecveResult *const result);
