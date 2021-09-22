#include "executant.h"
#include<sys/stat.h>
#include<fcntl.h> 
#include <sys/resource.h>
#include <seccomp.h>


//重定向三大文件
static void redrection(const char* const inputPath, const char* const outputPath, const char* const stderrPath);
//进行内存，时间，输出大小的限制
static void setResourceLimitation(const struct ExecveConfig* const);
//禁止除读写文件外的系统调用
static void forbidSyscall(const struct ExecveConfig* const config);

void startExecutant(const struct ExecveConfig const* config){
    redrection(config->stdinPath, config->stdoutPath, config->stderrPath);
    setResourceLimitation(config);
    // 设置uid
    if (config->userId != USER_ID_DEFAULT) {
        if (setuid(config->userId) == -1) {
            // printf("fail at setuid\n");
            _exit(UNABLE_TO_SET_UID);
        }
    }
    // printf("uid success\n");
    forbidSyscall(config);
    // printf("forbid seccuss\n");


    const char *env[] = {"PATH=/bin", NULL};
    execve(config->execvePath, NULL, env);
    _exit(0);
}

void redrection(const char *inputPath, const char *outputPath, const char* stderrPath){
    if(NULL != inputPath){
        const int inputNo = open(inputPath,O_RDONLY);
        if(-1 == inputNo)
            _exit(UNABLE_TO_GET_INPUT);
        dup2(inputNo, STDIN_FILENO);
    }
    if(NULL != outputPath){
        const int outputNo = open(outputPath,O_WRONLY);
        if(-1 == outputNo)
            _exit(UNABLE_TO_MAKE_OUTPUT);
        dup2(outputNo, STDOUT_FILENO);
    }
    if(NULL != stderrPath){
        const int errputNo = open(stderrPath,O_WRONLY);
        if(-1 == errputNo)
            _exit(UNABLE_TO_MAKE_OUTPUT);
        dup2(errputNo, STDERR_FILENO);
    }
}

void setResourceLimitation(const struct ExecveConfig* const config){
    struct rlimit limit;
    //限制内存（kB->B）
    limit.rlim_cur = config->memoryLimit * 1024;
    limit.rlim_max = config->wallMemoryLimit * 1024;
    if(0 != setrlimit(RLIMIT_AS, &limit))
        _exit(UNABLE_TO_LIMIT_MEM);
    //限制CPU时间（毫秒->秒）
    limit.rlim_cur = limit.rlim_max = config->cpuTimeLimit/1000 + 1;
    if(0 != setrlimit(RLIMIT_CPU, &limit))
        _exit(UNABLE_TO_LIMIT_CPU_TIME);
    //限制输出规模
    limit.rlim_cur = limit.rlim_max = config->outputSizeLimit;
    if(0 != setrlimit(RLIMIT_FSIZE, &limit))
        _exit(UNABLE_TO_LIMIT_OUTPUT);
    // 堆栈
    limit.rlim_cur = limit.rlim_max = RLIM_INFINITY;
    if (setrlimit(RLIMIT_STACK, &limit) != 0)
        _exit(UNABLE_TO_LIMIT_STACK);
}

inline void forbidSyscall(const struct ExecveConfig* const config){
    const static int FORBIDDEN_LIST[]={
    //静态常量，对提交程序开放的系统调用
        SCMP_SYS(fork),
        SCMP_SYS(clone),
        SCMP_SYS(vfork)
    };

    //初始化过滤器
    scmp_filter_ctx ctx;
    ctx = seccomp_init(SCMP_ACT_ALLOW);
    if(NULL == ctx){
        // printf("fail at init\n");
        _exit(UNABLE_TO_SECCOMP);
    }
    //添加禁止规则
    const size_t len = sizeof FORBIDDEN_LIST/sizeof(int);
    for(size_t i=0;i<len;i++)
        if(0 != seccomp_rule_add(ctx, SCMP_ACT_KILL, FORBIDDEN_LIST[i], 0)){
            // printf("fail at rule add\n");
            _exit(UNABLE_TO_SECCOMP);
        }
    
    //仅允许此进程execve提交程序
    const int ret = seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(execve), 1, SCMP_A0(SCMP_CMP_NE, (scmp_datum_t)(config->execvePath)));
    if(0 > ret){
        // printf("fail at exec add\n");
        _exit(UNABLE_TO_SECCOMP);
    }
    seccomp_load(ctx);
}