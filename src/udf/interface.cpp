/*
 * @Author: laihuihang laihuihang@foxmail.com
 * @Date: 2023-06-08 14:10:53
 * @LastEditors: laihuihang laihuihang@foxmail.com
 * @LastEditTime: 2023-06-15 16:27:05
 * @FilePath: /postgres-kernel/src/udf/interface.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "myfunc.h"
#include "interface.h"
#include "model/model_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

extern ModelManager model_manager;

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(RegisterCallback);
Datum RegisterCallback(PG_FUNCTION_ARGS)
{
    //model_manager.
    // model_manager.RegisterPreProcess("resnet", MyProcessImage);
    // model_manager.RegisterOutoutProcessFloat("resnet", MyOutPutProcessFloat);
    
    PG_RETURN_BOOL(true);
}

#ifdef __cplusplus
}
#endif