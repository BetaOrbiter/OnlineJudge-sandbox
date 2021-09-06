#include "common.h"

void initializeConfig(struct ExecveConfig *const config){
    config->cpuTimeLimit = CPU_TIME_LIMIT_DEFAULT;
    config->realTimeLimit = REAL_TIME_LIMIE_DEFAULT;
    config->memoryLimit = MEMORY_LIMIT_DEFAULT;
    config->wallMemoryLimit = WALL_MEMORY_LIMIT_DEFAULT;
    config->outputSizeLimit = OUTPUT_SIZE_LIMIT_DEFAULT;
    config->userId = USER_ID_DEFAULT;
    config->execvePath = NULL;
    config->stdinPath = NULL;
    config->stdoutPath = NULL;
    config->stderrPath = NULL;
}
void initializeResult(struct ExecveResult *const result){
    result->cpuTimeCost = 0;
    result->realTimeCost = 0;
    result->memoryCost = 0;
    result->condition = CONFIG_ERROR;
}
