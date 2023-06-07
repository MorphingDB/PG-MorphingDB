/**
 * 
 * 
 *  模型输出的接收类型
 * 
 *
 */

#include "postgres.h"

#include <math.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>

#include "libpq/pqformat.h"
#include "miscadmin.h"
#include "utils/float.h"
#include "utils/fmgrprotos.h"
#include "utils/model_res.h"


Datum
detc_res_in(PG_FUNCTION_ARGS)
{
    DETC_RES *result;
    char *str = PG_GETARG_CSTRING(0);

    result = (DETC_RES *) palloc(sizeof(DETC_RES));
    int nitems;


    /* Parse input string */
    nitems = sscanf(str, "(%d,%f,%d,%d,%d,%d)",
        &result->clazz,
        &result->confidence,
        &result->x1,
        &result->y1,
        &result->x2,
        &result->y2);
    if (nitems != 6)
        ereport(ERROR,
            (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                errmsg("invalid input syntax for detection_result: \"%s\"", str)));
    

    PG_RETURN_POINTER(result);
}


Datum
detc_res_out(PG_FUNCTION_ARGS)
{
    DETC_RES *result = (DETC_RES *) PG_GETARG_POINTER(0);
    char *result_str;


    /* Convert detection_result to string */
    result_str = psprintf("(%d,%f,%d,%d,%d,%d)",
        result->clazz,
        result->confidence,
        result->x1,
        result->y1,
        result->x2,
        result->y2);

    PG_RETURN_CSTRING(result_str);
}



Datum
detc_res_recv(PG_FUNCTION_ARGS)
{
    StringInfo  buf = (StringInfo) PG_GETARG_POINTER(0);
    DETC_RES *result;

    result->clazz = pq_getmsgint(buf, 4);
    result->confidence = pq_getmsgfloat4(buf);
    result->x1 = pq_getmsgint(buf, 4);
    result->y1 = pq_getmsgint(buf, 4);
    result->x2 = pq_getmsgint(buf, 4);
    result->y2 = pq_getmsgint(buf, 4);



    PG_RETURN_POINTER(result);
}


Datum
detc_res_send(PG_FUNCTION_ARGS)
{
    DETC_RES *result = (DETC_RES *) PG_GETARG_POINTER(0);
    StringInfoData buf;

    pq_begintypsend(&buf);
    pq_sendint32(&buf, result->clazz);
    pq_sendfloat4(&buf, result->confidence);
    pq_sendint32(&buf, result->x1);
    pq_sendint32(&buf, result->y1);
    pq_sendint32(&buf, result->x2);
    pq_sendint32(&buf, result->y2);

    PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}


Datum
detc_res_clazz(PG_FUNCTION_ARGS)
{
    DETC_RES *detection_result = (DETC_RES *) PG_GETARG_POINTER(0);

    PG_RETURN_INT32(detection_result->clazz);
}

Datum
detc_res_confidence(PG_FUNCTION_ARGS)
{
    DETC_RES *detc_res = (DETC_RES *) PG_GETARG_POINTER(0);
    PG_RETURN_FLOAT4(detc_res->confidence);
}


