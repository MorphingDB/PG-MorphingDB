#include "vector.h"
#include "fmgr.h"



#define DatumGetVector(x_) ((Vector *) PG_DETOAST_DATUM(x_))

#define PG_GETARG_VECTOR_P(x_) DatumGetVector(PG_GETARG_DATUM(x_))