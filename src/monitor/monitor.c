#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

#include "monitor.h"
#include "../executant/executant.h"

struct RealTimeKillerConfig
{
    pid_t pid;
    unsigned realTimeLimit;
};
//fork后启动该线程，定时杀死，防止提交程序sleep卡死，逃过setrlimit
void *realTimeKiller(void *realTimeKillerConfig);

//根据进程消耗资源、推出情况设置执行结果
static enum RUNNING_CONDITION setRunningCondition(
    const int status, const struct ExecveConfig*, const struct ExecveResult*);
//顾名思义
static unsigned long getMillisecond(const struct timeval val);

void startMonitor(const struct ExecveConfig* const config, struct ExecveResult* const result){
    struct timeval startTime, endTime;
    if(0 != getuid()){
        // printf("fail ai root user\n");
        //需要root权限对提交程序进行资源限制
        result->condition = UNABLE_TO_SET_UID;
        return ;
    }
    
    gettimeofday(&startTime, NULL);
    pid_t childPid = fork();
    if(childPid < 0){
        // printf("fail at fork\n");
        //创建子进程失败
        result->condition = UNABLE_TO_EXECVE;
        return;
    }

    if(0 == childPid){
        //子进程设置限制，执行程序
        startExecutant(config);
    }
    else{
        //父进程监视收集子进程资源
        pthread_t killerThreadId;
        struct RealTimeKillerConfig realTimeKillerConfig;
        realTimeKillerConfig.pid = childPid;
        realTimeKillerConfig.realTimeLimit = config->realTimeLimit;
        const int ret = pthread_create(&killerThreadId, NULL, realTimeKiller,(void*) &realTimeKillerConfig);
        if(0 != ret){
            printf("fail at time killer\n");
            result->condition = UNABLE_TO_LIMIT_REAL_TIME;
            kill(childPid, SIGKILL);
            return;
        }

        int status;
        struct rusage costs;
        wait4(childPid, &status, WSTOPPED, &costs);
        gettimeofday(&endTime, NULL);
        pthread_cancel(killerThreadId);

        //设置结果
        result->cpuTimeCost = getMillisecond(costs.ru_utime);
        result->realTimeCost = getMillisecond(endTime) - getMillisecond(startTime);
        result->memoryCost = costs.ru_maxrss;
        result->condition = setRunningCondition(status, config, result);
    }
}

void *realTimeKiller(void *realTimeKillerConfig){
    struct RealTimeKillerConfig *config = realTimeKillerConfig;
    sleep(config->realTimeLimit);
    kill(config->pid, SIGKILL);
    return NULL;
}


inline unsigned long getMillisecond(const struct timeval val){
    return val.tv_sec*1000 + val.tv_usec/1000;
}

enum RUNNING_CONDITION setRunningCondition(
    const int status, const struct ExecveConfig* config, const struct ExecveResult* result){
    
    //正常退出
    if (WIFEXITED(status)) {
        if (0 == WEXITSTATUS(status)) {
            // 细粒度判断内存限制，通过杀进程的方式并不准确，详见child.c文件
            bool isMemoryExceeded = (unsigned long long) (result->memoryCost) > config->memoryLimit;
            if (isMemoryExceeded) {
                return MEMORY_LIMIT_EXCEED;
            }

            // 细粒度的时间限制(ms)
            int isCpuTimeExceeded = config->cpuTimeLimit < result->cpuTimeCost;
            int isRealTimeExceeded = config->realTimeLimit < result->realTimeCost;
            if (isCpuTimeExceeded || isRealTimeExceeded) {
                return TIME_LIMIT_EXCEED;
            }
            return SUCCESS;
        }
        return WEXITSTATUS(status);
    }

    // 异常终止
    if (WIFSIGNALED(status)) {
        if (WTERMSIG(status) == SIGXCPU) {
            return TIME_LIMIT_EXCEED;
        }
        if (WTERMSIG(status) == SIGSEGV) {
            return SEGMENTATION_FAULT;
        }
        if (WTERMSIG(status) == SIGKILL) {
            // 经测试 cpu的时间超限也会出现在此处
            if (config->cpuTimeLimit < result->cpuTimeCost) {
                return TIME_LIMIT_EXCEED;
            }
            return RUNTIME_ERROR;
        }
        if (WTERMSIG(status) == SIGXFSZ) {
            return OUTPUT_LIMIT_EXCEED;
        }
        return RUNTIME_ERROR;
    }
    return UNKNOWN_ERROR;
}