#include "postgres.h"

#include "catalog/pg_type_d.h"
#include "lib/stringinfo.h"
#include "libpq/pqformat.h"
#include "utils/builtins.h"
#include "utils/array.h"
#include "utils/vector.h"


#define INIT_VECTOR 

static inline bool 
is_space(char ch)
{
    if (ch == ' ' ||
		ch == '\t' ||
		ch == '\n' ||
		ch == '\r' ||
		ch == '\v' ||
		ch == '\f')
		return true;
	return false;
}


static inline void 
parse_vector_str(char *str, unsigned int* dim, float* x)
{
    char* str_copy = pstrdup(str);
    while(is_space(*str)){
        str++;
    }

    if (*str != '['){
        ereport(ERROR,
				 errmsg("Vector contents must start with \"[\"."));
    }

    str++;
    char* pt = strtok(str, ",");
    char* end = pt;

    while (pt != NULL && *end != ']') {
        if (*dim == MAX_VECTOR_DIM) {
            ereport(ERROR,
					(errmsg("vector cannot have more than %d dimensions", MAX_VECTOR_DIM)));
        }

        while (is_space(*pt)){
            pt++;
        }

        if (*pt == '\0') {
            ereport(ERROR,
					(errmsg("invalid input syntax for type vector: \"%s\"", str_copy)));
        }

        x[*dim] = strtof(pt, &end);

        (*dim)++;

        if(end == pt){
            ereport(ERROR,
					(errmsg("invalid input syntax for type vector: \"%s\"", str_copy)));
        }

        while(is_space(*end)){
            end++;
        }

        if (*end != '\0' && *end != ']'){
            ereport(ERROR,
					(errmsg("invalid input syntax for type vector: \"%s\"", str_copy)));
        }

        pt = strtok(NULL, ",");
    }

    if (end == NULL || *end != ']'){
        ereport(ERROR,
					(errmsg("malformed vector literal: \"%s\"", str_copy)));
    }

    end++;

    while (is_space(*end)){
        end++;
    }

    if (*end != '\0'){
        ereport(ERROR,
					(errmsg("malformed vector literal: \"%s\"", str_copy)));
    }

    for(pt = str_copy + 1; *pt != '\0'; pt++){
        if (pt[-1] == ',' && *pt == ','){
            ereport(ERROR,
					(errmsg("malformed vector literal: \"%s\"", str_copy)));
        }
    }

    if(*dim < 1){
        ereport(ERROR,
					(errmsg("vector must have at least 1 dimension")));
    }

    pfree(str_copy);

}

Datum
vector_input(PG_FUNCTION_ARGS)
{
    char            *str = NULL;
    float           x[MAX_VECTOR_DIM];
    Vector          *vector = NULL;
    unsigned int    dim = 0;

    str = PG_GETARG_CSTRING(0);

    parse_vector_str(str, &dim, x);

    vector = (Vector*)palloc(VECTOR_SIZE(dim));
    SET_VARSIZE(vector, VECTOR_SIZE(dim));

    vector->dim = dim;
    for(int i=0; i<dim; ++i){
        vector->x[i] = x[i];
    }

    PG_RETURN_POINTER(vector);
}

Datum
vector_output(PG_FUNCTION_ARGS)
{
    Vector* vector = NULL;
    StringInfoData result;
    unsigned int dim = 0;

    vector = PG_GETARG_VECTOR_P(0);

    dim = VECTOR_DIM(vector);

    if(dim > MAX_VECTOR_DIM){
        ereport(ERROR,
					(errmsg("dim is larger than 10240 dim!")));
    }

    initStringInfo(&result);
    appendStringInfoChar(&result, '[');
    for(int i=0; i<dim; ++i){
        appendStringInfoString(&result, DatumGetCString(DirectFunctionCall1(float4out, Float4GetDatum(vector->x[i]))));
        if (i != dim - 1) {
            appendStringInfoChar(&result, ',');
        }
    }
    appendStringInfoChar(&result, ']');

    PG_RETURN_CSTRING(result.data);
}

Datum
vector_receive(PG_FUNCTION_ARGS)
{
    StringInfo        str;
    Vector*           result = NULL;
    unsigned int      dim = 0;
    int               vector_size;

    str = (StringInfo)PG_GETARG_POINTER(0);
    dim = pq_getmsgint(str, sizeof(unsigned int));

    vector_size = VECTOR_SIZE(dim);

    result = (Vector *)palloc(vector_size);
    SET_VARSIZE(result, vector_size);

    result->dim = dim;
    for(int i=0; i<dim; ++i){
        result->x[i] = pq_getmsgfloat4(str);
    }

    PG_RETURN_POINTER(result);
}

Datum
vector_send(PG_FUNCTION_ARGS)
{
    Vector	        *vector = PG_GETARG_VECTOR_P(0);
	StringInfoData  str;

	pq_begintypsend(&str);
	pq_sendint(&str, vector->dim, sizeof(unsigned int));
	for (int i = 0; i < vector->dim; ++i){
        pq_sendfloat4(&str, vector->x[i]);
    }

	PG_RETURN_BYTEA_P(pq_endtypsend(&str));
}

Datum
array_to_vector(PG_FUNCTION_ARGS)
{
    ArrayType       *array = NULL;
    Oid             array_type;
    Vector          *vector = NULL;
    int             array_length;
    unsigned int    dim = 0;
    Datum           *elems = NULL;
    bool            *nulls = NULL;
    int16		    typlen = 0;
	bool		    typbyval = 0;
	char		    typalign = 0;


    array = PG_GETARG_ARRAYTYPE_P(0);

    //get_typlenbyvalalign(ARR_ELEMTYPE(array), &typlen, &typbyval, &typalign);

    array_type = ARR_ELEMTYPE(array);

    
    switch (array_type) {
        case FLOAT4OID:
        {
            deconstruct_array(array, FLOAT4OID, sizeof(float4), true, 'i', &elems, &nulls, &array_length);
            break;
        }
        case FLOAT8OID:
        {
            deconstruct_array(array, FLOAT8OID, sizeof(float8), FLOAT8PASSBYVAL, 'd', &elems, &nulls, &array_length);
            for(int i=0; i<array_length; ++i){
                elems[i] = Float4GetDatum(DatumGetFloat8(elems[i]));
            }
            break;
        }
        case INT4OID:
        {
            deconstruct_array(array, INT4OID, sizeof(int), true, 'i', &elems, &nulls, &array_length);
            for(int i=0; i<array_length; ++i){
                elems[i] = Float4GetDatum(DatumGetInt32(elems[i]));
            }
            break;
        }
        default:
        {
            ereport(ERROR,
					(errmsg("unsupport %d type to vector!"), array_type));
        }
    }

    if(array_length > MAX_VECTOR_DIM){
        ereport(ERROR,
					(errmsg("vector cannot have more than %d dimensions", MAX_VECTOR_DIM)));
    }

    vector = (Vector *)palloc(VECTOR_SIZE(array_length));
    SET_VARSIZE(vector, VECTOR_SIZE(array_length));

    vector->dim = array_length;
    for(int i=0; i<array_length; ++i){
        vector->x[i] = DatumGetFloat4(elems[i]);
    }

    PG_RETURN_POINTER(vector);

}


Datum
text_to_vector(PG_FUNCTION_ARGS)
{
    char             *str = NULL;
    Vector           *vector = NULL;
    unsigned int     dim = 0;
    float            x[MAX_VECTOR_DIM];

    str = TextDatumGetCString(PG_GETARG_DATUM(0));

    parse_vector_str(str, &dim, x);

    vector = (Vector*)palloc(VECTOR_SIZE(dim));
    SET_VARSIZE(vector, VECTOR_SIZE(dim));

    vector->dim = dim;
    for(int i=0; i<dim; ++i){
        vector->x[i] = x[i];
    }

    PG_RETURN_POINTER(vector);
}

Datum
get_vector(PG_FUNCTION_ARGS)
{
    Vector	        *vector = NULL;
    float           x[MAX_VECTOR_DIM];
    ArrayType       *result = NULL;
    Datum           *elems = NULL; 
    unsigned int    dim = 0;
    
    vector = PG_GETARG_VECTOR_P(0);

    dim = vector->dim;

    elems = (Datum *)palloc(sizeof(Datum *) * dim);

    for(int i=0; i<dim; ++i){
        elems[i] = Float4GetDatum(vector->x[i]);
    }

    result = construct_array(elems, dim, FLOAT4OID, sizeof(float4), true, 'i');

    pfree(elems);
    PG_RETURN_ARRAYTYPE_P(result);
}

Datum
vector_add(PG_FUNCTION_ARGS)
{
    Vector	        *vector_left = NULL;
    Vector          *vector_right = NULL;
    unsigned int    dim = 0;
    Vector          *result = NULL;

    vector_left = PG_GETARG_VECTOR_P(0);
    vector_right = PG_GETARG_VECTOR_P(1);


    if(vector_left->dim != vector_right->dim){
        ereport(ERROR,
					(errmsg("the two vectors have different dimensions!")));
    }

    dim = vector_left->dim;

    result = (Vector*)palloc(VECTOR_SIZE(dim));
    SET_VARSIZE(result, VECTOR_SIZE(dim));

    result->dim = dim;

    for(int i=0; i<dim; ++i){
        result->x[i] = vector_left->x[i] + vector_right->x[i];
        if(isinf(result->x[i])){
            ereport(ERROR,
					(errmsg("value out of range: overflow!")));
        }
    }

    PG_RETURN_POINTER(result);
}

Datum
vector_sub(PG_FUNCTION_ARGS)
{
    Vector	        *vector_left = NULL;
    Vector          *vector_right = NULL;
    unsigned int    dim = 0;
    Vector          *result = NULL;

    vector_left = PG_GETARG_VECTOR_P(0);
    vector_right = PG_GETARG_VECTOR_P(1);


    if(vector_left->dim != vector_right->dim){
        ereport(ERROR,
					(errmsg("the two vectors have different dimensions!")));
    }

    dim = vector_left->dim;

    result = (Vector*)palloc(VECTOR_SIZE(dim));
    SET_VARSIZE(result, VECTOR_SIZE(dim));

    result->dim = dim;

    for(int i=0; i<dim; ++i){
        result->x[i] = vector_left->x[i] - vector_right->x[i];
        if(isinf(result->x[i])){
            ereport(ERROR,
					(errmsg("value out of range: overflow!")));
        }
    }

    PG_RETURN_POINTER(result);
}

Datum
vector_equal(PG_FUNCTION_ARGS)
{
    Vector	        *vector_left = NULL;
    Vector          *vector_right = NULL;

    vector_left = PG_GETARG_VECTOR_P(0);
    vector_right = PG_GETARG_VECTOR_P(1);


    if(vector_left->dim != vector_right->dim){
        PG_RETURN_BOOL(false);
    }

    for(int i=0; i<vector_left->dim; ++i){
        if(vector_left->x[i] != vector_right->x[i]){
            PG_RETURN_BOOL(false);
        }
    }

    PG_RETURN_BOOL(true);
}