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
    int32   clazz;
    float4 confidence; 
    int32    x1,y1, x2,y2;


} DETC_RES;


#endif
