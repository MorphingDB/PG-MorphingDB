/*
 * @Author: laihuihang laihuihang@foxmail.com
 * @Date: 2023-06-05 15:36:54
 * @LastEditors: laihuihang laihuihang@foxmail.com
 * @LastEditTime: 2023-06-19 10:33:30
 * @FilePath: /postgres-kernel/src/backend/utils/adt/predict.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "postgres.h"
#include "fmgr.h"

#include "model/predict_wrapper.h"
#include "utils/builtins.h"
#include "catalog/pg_type_d.h"


// args are "state, model, cuda, vector_elements.."
#define VECTOR_START_ARG_INDEX 3

Datum
pg_predict_batch_accum(PG_FUNCTION_ARGS)
{
    VecAggState*    state;
    MemoryContext old_context;
    Args*           vec;

    state = PG_ARGISNULL(0) ? NULL : (VecAggState *) PG_GETARG_POINTER(0);
    
    /* Create the state data on the first call */
    if (state == NULL)
        state = makeVecAggState(fcinfo);

    old_context = MemoryContextSwitchTo(state->ctx);

    vec = makeVecFromArgs(fcinfo, VECTOR_START_ARG_INDEX, PG_NARGS() - VECTOR_START_ARG_INDEX);
    state->ins = lappend(state->ins, vec);

    MemoryContextSwitchTo(old_context);

    PG_RETURN_POINTER(state);
}

Datum
pg_predict_batch_accum_inv(PG_FUNCTION_ARGS) 
{
    VecAggState*    state;
    
    state = (VecAggState *) PG_GETARG_POINTER(0);

    /* finilize has been called */
    Assert((state != NULL) && (state->prcsd_batch_n != 0));

    if (state->nxt_csr == state->prcsd_batch_n) 
    {
        for (int i = 0; i < state->prcsd_batch_n; i++) 
        {
            state->ins = list_delete_first(state->ins);
            state->outs = list_delete_first(state->outs);
        }
        state->nxt_csr -= state->prcsd_batch_n;
        state->prcsd_batch_n -= state->prcsd_batch_n;
    }

    PG_RETURN_POINTER(state);
}

static Args*
fetch_next_from_predicted_batch(PG_FUNCTION_ARGS, bool ret_float8) 
{
    VecAggState*    state = NULL;
    Args*           ret = NULL;
    MemoryContext   old_context;

    state = PG_ARGISNULL(0) ? NULL : (VecAggState *) PG_GETARG_POINTER(0);

    /* If there were no non-null inputs, return NULL */
    if (state == NULL || list_length(state->ins) == 0)
        return NULL;

    /* do batch infer once if need */
    if (state->nxt_csr >= state->prcsd_batch_n)
    {
        old_context = MemoryContextSwitchTo(state->ctx);
        infer_batch_internal(state, ret_float8);
        MemoryContextSwitchTo(old_context);

        if (Debug_print_batch_time)
        {
            int64_t total = state->pre_time + state->infer_time + state->post_time;
            ereport(NOTICE, 
                (errmsg("\nbatch %d:\n"
                        " pre process: %ld ms(%.2f%%)\n"
                        " infer: %ld ms(%.2f%%)\n"
                        " post process: %ld ms(%.2f%%)",
                        state->batch_i, 
                        state->pre_time, (state->pre_time / (float)total) * 100, 
                        state->infer_time, (state->infer_time / (float)total)  * 100, 
                        state->post_time, (state->post_time / (float)total  * 100))));
        }
    }

    /* consume one result */ 
    state = (VecAggState *) PG_GETARG_POINTER(0);
    ret = (Args*)list_nth(state->outs, state->nxt_csr);
    state->nxt_csr++;

    return ret;
}

Datum
pg_predict_batch_final_float(PG_FUNCTION_ARGS)
{
    Args* final_ret = fetch_next_from_predicted_batch(fcinfo, true);

    if (final_ret == NULL)
        PG_RETURN_NULL();

    PG_RETURN_FLOAT8(final_ret->floating);
}

Datum
pg_predict_batch_final_text(PG_FUNCTION_ARGS)
{
    Args* final_ret = fetch_next_from_predicted_batch(fcinfo, false);

    if (final_ret == NULL)
        PG_RETURN_NULL();

    PG_RETURN_TEXT_P(cstring_to_text(final_ret->ptr));
}

/**
 * @description: pg预测系统函数, 返回值为float, 前两个参数固定，后面为可变参数
 * @event: 
 * @return {*}
 */
Datum 
pg_predict_float(PG_FUNCTION_ARGS)
{
    char*           model_name = NULL; 
    char*           cuda = NULL;

    Args*           args = (Args*)palloc((PG_NARGS()-2) * sizeof(Args)); 

    Datum           ret;

    model_name = PG_GETARG_CSTRING(0);
    cuda       = PG_GETARG_CSTRING(1);

    // 根据传入的列数生成参数
    for (int i = 2; i < PG_NARGS(); i++) {
        switch (get_fn_expr_argtype(fcinfo->flinfo, i)) {
            case INT4OID:
            case INT2OID:
            case INT8OID:
            {
                int cur_int = PG_GETARG_INT32(i);
                args[i-2].integer = cur_int;
                break;
            }
            case FLOAT4OID:
            case FLOAT8OID:
            {
                float8 cur_float = PG_GETARG_FLOAT8(i);
                args[i-2].floating = cur_float;
                break;
            }
            case TEXTOID:
            {
                char* cur_text = TextDatumGetCString(PG_GETARG_DATUM(i));
                args[i-2].ptr = cur_text;
                break;
            }
            case CSTRINGOID:
            {
                char* cur_cstring = PG_GETARG_CSTRING(i);
                args[i-2].ptr = cur_cstring;
                break;
            }
            case NUMERICOID:
            {
                Datum numer = PG_GETARG_DATUM(i);
                float8 num_float = DatumGetFloat8(DirectFunctionCall1(numeric_float8, numer));;
                args[i-2].floating = num_float;
                break;
            }
            default:
            {
                ereport(ERROR, (errmsg("%d type don't support!", get_fn_expr_argtype(fcinfo->flinfo, i))));
                break;
            }
        }
    }

    ret = Float8GetDatum(predict_float(model_name, cuda, args));
    pfree(args);
    PG_RETURN_DATUM(ret);
}


/**
 * @description: pg预测系统函数，返回值为text，前两个参数固定，后面为可变参数
 * @event: 
 * @return {*}
 */
Datum 
pg_predict_text(PG_FUNCTION_ARGS)
{
    char*           model_name = NULL; 
    char*           cuda = NULL;

    Args*           args = (Args*)palloc((PG_NARGS()-2) * sizeof(Args)); 

    Datum           ret;

    model_name = PG_GETARG_CSTRING(0);
    cuda       = PG_GETARG_CSTRING(1);

    // 根据传入的列数生成参数
    for (int i = 2; i < PG_NARGS(); i++) {
        switch (get_fn_expr_argtype(fcinfo->flinfo, i)) {
            case INT4OID:
            case INT2OID:
            case INT8OID:
            {
                int cur_int = PG_GETARG_INT32(i);
                args[i-2].integer = cur_int;
                break;
            }
            case FLOAT4OID:
            case FLOAT8OID:
            {
                float8 cur_float = PG_GETARG_FLOAT8(i);
                args[i-2].floating = cur_float;
                break;
            }
            case TEXTOID:
            {
                char* cur_text = TextDatumGetCString(PG_GETARG_DATUM(i));
                args[i-2].ptr = cur_text;
                break;
            }
            case CSTRINGOID:
            {
                char* cur_cstring = PG_GETARG_CSTRING(i);
                args[i-2].ptr = cur_cstring;
                break;
            }
            case NUMERICOID:
            {
                Datum numer = PG_GETARG_DATUM(i);
                float8 num_float = DatumGetFloat8(DirectFunctionCall1(numeric_float8, numer));;
                args[i-2].floating = num_float;
                break;
            }
            default:
            {
                ereport(ERROR, (errmsg("%d type don't support!", get_fn_expr_argtype(fcinfo->flinfo, i))));
                break;
            }
        }
    }
    
    ret = PointerGetDatum(predict_text(model_name, cuda, args));
    pfree(args);
    PG_RETURN_TEXT_P(ret);
}