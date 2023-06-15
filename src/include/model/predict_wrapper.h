/*
 * @Author: laihuihang laihuihang@foxmail.com
 * @Date: 2023-06-02 16:28:48
 * @LastEditors: laihuihang laihuihang@foxmail.com
 * @LastEditTime: 2023-06-15 15:26:08
 * @FilePath: /postgres-kernel/src/include/model/interface.h
 * @Description: 
 */
#ifndef _PREDICT_WRAPPER_H_
#define _PREDICT_WRAPPER_H_


#include "model_define.h"
#ifdef __cplusplus
extern "C" {
#endif

float8 predict_float(const char* model_name, const char* cuda, Args* args);

text*  predict_text(const char* model_name, const char* cuda, Args* args);


#ifdef __cplusplus
}
#endif


#endif