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
    nitems = sscanf(str, "(%d,%f,((%f,%f),(%f,%f)))",
        &result->clazz,
        &result->confidence,
        &result->location.low.x,
        &result->location.low.y,
        &result->location.high.x,
        &result->location.high.y);
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
    result_str = psprintf("(%d,%f,((%f,%f),(%f,%f)))",
        result->clazz,
        result->confidence,
        result->location.low.x,
        result->location.low.y,
        result->location.high.x,
        result->location.high.y);

    PG_RETURN_CSTRING(result_str);
}



Datum
detc_res_recv(PG_FUNCTION_ARGS)
{
    StringInfo  buf = (StringInfo) PG_GETARG_POINTER(0);
    DETC_RES *result;

    result = (DETC_RES *) palloc(sizeof(DETC_RES));
    result->clazz = pq_getmsgint(buf, sizeof(int));
    result->confidence = pq_getmsgfloat4(buf);
    result->location.low.x = pq_getmsgfloat8(buf);
    result->location.low.y = pq_getmsgfloat8(buf);
    result->location.high.x = pq_getmsgfloat8(buf);
    result->location.high.y = pq_getmsgfloat8(buf);

    PG_RETURN_POINTER(result);
}


Datum
detc_res_send(PG_FUNCTION_ARGS)
{
    DETC_RES *result = (DETC_RES *) PG_GETARG_POINTER(0);
    StringInfoData buf;

    pq_begintypsend(&buf);
    pq_sendint(&buf, result->clazz, sizeof(int));
    pq_sendfloat4(&buf, result->confidence);
    pq_sendfloat8(&buf, result->location.low.x);
    pq_sendfloat8(&buf, result->location.low.y);
    pq_sendfloat8(&buf, result->location.high.x);
    pq_sendfloat8(&buf, result->location.high.y);

    PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}
