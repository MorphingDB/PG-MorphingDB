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

#ifdef __cplusplus
extern "C" {
#endif

#include "postgres.h"

#include "fmgr.h"
#include "model_define.h"
#include "nodes/pg_list.h"
#include "utils/palloc.h"
#include "utils/vector.h"

extern bool Debug_print_batch_time;

typedef struct VecAggState {
    MemoryContext ctx;
    List* ins;
    List* outs;
    int batch_i;
    int prcsd_batch_n;
    char* model;
    char* cuda;
    int nxt_csr;
    int64_t pre_time;   // ms
    int64_t infer_time; // ms
    int64_t post_time;  // ms
} VecAggState;


typedef struct ModelLayer {
    char*    layer_name;
    Vector*  layer_parameter;
} ModelLayer;

VecAggState *makeVecAggState(FunctionCallInfo fcinfo);

Args* makeVecFromArgs(FunctionCallInfo fcinfo, int start, int dim);

void infer_batch_internal(VecAggState* state, bool ret_float8);

float8 predict_float(const char* model_name, const char* cuda, Args* args);

text*  predict_text(const char* model_name, const char* cuda, Args* args);

void   model_parameter_extraction(const char* model_path, ModelLayer** parameter_list, uint32* layer_size);




#ifdef __cplusplus
}
#endif


#endif