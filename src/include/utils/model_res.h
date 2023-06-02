#ifndef MODEL_RES_H
#define MODEL_RES_H

#include "fmgr.h"
#include "c.h"
#include "utils/geo_decls.h"


/**
 * 目标检测的输出包装类
*/
typedef struct  
{
    int8   clazz;
    float8 confidence; 
    BOX    location; 

} DETC_RES;


#endif
