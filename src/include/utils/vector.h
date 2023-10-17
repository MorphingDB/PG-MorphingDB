#ifndef VECTOR_H
#define VECTOR_H


#include <c.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_VECTOR_DIM (1024000)
#define MAX_VECTOR_SHAPE_SIZE (10)

typedef struct Vector
{
    int32 vl_len_;
    unsigned int dim;
    unsigned int shape_size;
    int32 shape[MAX_VECTOR_SHAPE_SIZE];
    float x[FLEXIBLE_ARRAY_MEMBER];
} Vector;

#define VECTOR_SIZE(dim_) (offsetof(Vector, x) + sizeof(int32)*(MAX_VECTOR_SHAPE_SIZE) + sizeof(float)*(dim_))
#define VECTOR_DATA_SIZE(dim_) (sizeof(float) * (dim))

#define VECTOR_DIM(x_) ((x_)->dim)

#define VECTOR_SHAPE_SIZE(x_) ((x_)->shape_size)

#define DatumGetVector(x_) ((Vector *) PG_DETOAST_DATUM(x_))

#define PG_GETARG_VECTOR_P(x_) DatumGetVector(PG_GETARG_DATUM(x_))


// vector_tensor.cpp
Vector* new_vector(unsigned int dim, unsigned int shape);
void free_vector(Vector* vector);

#ifdef __cplusplus
}
#endif

#endif