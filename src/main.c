#include <stdio.h>
#include <stdbool.h>
#include "common/common.h"
#include "./monitor/monitor.h"

static void getAndSetConfig(int argc, char *argv[], struct ExecveConfig *config);
static bool checkConfig(const struct ExecveConfig *const config);
static void printResult(const struct ExecveResult *const result);
static void showUsage(void);

int main(int argc, char *argv[]){
    struct ExecveConfig config;
    struct ExecveResult result;
    initializeConfig(&config);
    initializeResult(&result);

    getAndSetConfig(argc, argv, &config);
    if(checkConfig(&config))
        startMonitor(&config, &result);
    else
        result.condition = CONFIG_ERROR;
    
    printResult(&result);
    return 0;
}


void getAndSetConfig(int argc, char *argv[], struct ExecveConfig *config){
    if(1 == argc){
        showUsage();
        return;
    }

    int opt;
    while(-1 != (opt = getopt(argc, argv, "t:c:m:f:o:e:i:r:u:h:"))){
        switch (opt) {
            case 't':
                config->realTimeLimit = atoi(optarg);
                break;
            case 'c':
                config->cpuTimeLimit = atoi(optarg);
                break;
            case 'm':
                config->memoryLimit = atoi(optarg);
                break;
            case 'f':
                config->outputSizeLimit = atoi(optarg);
                break;
            case 'o':
                config->stdoutPath = optarg;
                break;
            case 'e':
                config->stderrPath = optarg;
                break;
            case 'i':
                config->stdinPath = optarg;
                break;
            case 'r':
                config->execvePath = optarg;
                break;
            case 'u':
                config->userId = atoi(optarg);
                break;
            case 'h':
                showUsage();
                return ;
            default:
                printf("Unknown option: %c\n", (char) optopt);
                return ;
        }
    }
}
bool checkConfig(const struct ExecveConfig *const config){
    if(config->cpuTimeLimit < 0
       || config->realTimeLimit < 0
       || config->memoryLimit < 1024
       || config->wallMemoryLimit < 1024
       || config->outputSizeLimit < 0
       || config->execvePath == NULL
       || config->execvePath[0] == '\0')
        return false;
    else
        return true;
}
inline void printResult(const struct ExecveResult *const result) {
    // 此处的stdout将被调用者处理 应该以json字符串形式表示
    printf("%d %d %d", result->cpuTimeCost, result->memoryCost, result->condition);
}

inline void showUsage(void){
    printf("\n[限制相关]\n\
  -t,     限制实际时间为t毫秒，请注意和cpu时间区分\n\
  -c,     限制cpu时间为t毫秒\n\
  -m,     限制运行内存为mKB\n\
  -f,     限制代码最大输出为fB\n\
  -u,     执行程序的用户id\n");

    printf("[输入/输出相关]\n\
  -r,     目标可执行文件\n\
  -o,     标准输出文件\n\
  -e,     标准错误文件\n\
  -i,     标准输入文件\n");

    printf("[其他]\n\
  -h,     查看帮助\n\n");

}