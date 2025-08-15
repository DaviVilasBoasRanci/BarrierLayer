#include "../include/path_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static char log_path[PATH_MAX];
static int path_initialized = 0;

const char* get_log_path(void) {
    if (!path_initialized) {
        const char* home_dir = getenv("HOME");
        if (home_dir) {
            snprintf(log_path, sizeof(log_path), "%s/BarrierLayer/barrierlayer_activity.log", home_dir);
        } else {
            // Fallback para o diretório atual se HOME não estiver definido
            snprintf(log_path, sizeof(log_path), "./barrierlayer_activity.log");
        }
        path_initialized = 1;
    }
    return log_path;
}
