#ifndef VECTOR_H
#define VECTOR_H

#include "fmgr.h"

typedef struct Vector
{
    int32 vl_len_;
    unsigned int dim;
    float x[FLEXIBLE_ARRAY_MEMBER];
} Vector;

#define MAX_VECTOR_DIM (1024000)

#define VECTOR_SIZE(dim_) (offsetof(Vector, x) + sizeof(float) * (dim_))

#define VECTOR_DIM(x_) ((x_)->dim)

#define DatumGetVector(x_) ((Vector *) PG_DETOAST_DATUM(x_))

#define PG_GETARG_VECTOR_P(x_) DatumGetVector(PG_GETARG_DATUM(x_))

#endif