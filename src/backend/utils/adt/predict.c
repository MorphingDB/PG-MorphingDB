/*
 * @Author: laihuihang laihuihang@foxmail.com
 * @Date: 2023-06-05 15:36:54
 * @LastEditors: laihuihang laihuihang@foxmail.com
 * @LastEditTime: 2023-06-15 09:42:07
 * @FilePath: /postgres-kernel/src/backend/utils/adt/predict.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "postgres.h"
#include "fmgr.h"

#include "model/predict_wrapper.h"
#include "utils/builtins.h"
#include "catalog/pg_type_d.h"





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