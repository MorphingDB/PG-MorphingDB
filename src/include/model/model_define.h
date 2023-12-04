#ifndef _DEFINE_H_
#define _DEFINE_H_

#include "c.h"

typedef union {
    void* ptr;
    int   integer;
    float8 floating;
} Args;

void register_default_model();
char* replace_model_path(char* origin_path);

#endif