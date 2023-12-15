#include "postgres.h"

#include "catalog/pg_type_d.h"
#include "lib/stringinfo.h"
#include "libpq/pqformat.h"
#include "utils/builtins.h"
#include "utils/array.h"
#include "utils/palloc.h"
#include "utils/vector.h"
#include <stdbool.h>

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


Vector* 
new_vector(unsigned int dim, unsigned int shape_size)
{
    Vector* vector;
    
    if(dim >= MAX_VECTOR_DIM){

    }

    if(shape_size >= MAX_VECTOR_SHAPE_SIZE){

    }

    vector = (Vector*)palloc(VECTOR_SIZE(dim));
    vector->dim = dim;
    vector->shape_size = shape_size;

    SET_VARSIZE(vector, VECTOR_SIZE(dim));

    return vector;
}

void 
free_vector(Vector* vector)
{
    if(vector == NULL){
        return;
    }
    pfree(vector);
    vector = NULL;
}

static inline bool
shape_equal(Vector* vector_left, Vector* vector_right)
{
    if(vector_left == NULL || vector_right == NULL){
        return false;
    }

    if(vector_left->shape_size != vector_right->shape_size){
        return false;
    }

    for(int i=0; i<vector_left->shape_size; ++i){
        if(vector_left->shape[i] != vector_right->shape[i]){
            return false;
        }
    }
    return true;
}

/*
    parse vector shape
*/
static inline void 
parse_vector_shape_str(char* shape_str, unsigned int* shape_size, int* shape)
{
    char* str_copy = pstrdup(shape_str);
    char* pt = NULL;
    char* end = NULL;

    while(is_space(*shape_str)){
        shape_str++;
    }

    if (*shape_str != '{'){
        ereport(ERROR,
				 errmsg("Vector shape must start with \"{\"."));
    }

    shape_str++;
    pt = strtok(shape_str, ",");
    end = pt;

    while (pt != NULL && *end != '}') {
        if (*shape_size == MAX_VECTOR_SHAPE_SIZE) {
            ereport(ERROR,
					(errmsg("vector shape cannot have more than %d", MAX_VECTOR_SHAPE_SIZE)));
        }

        while (is_space(*pt)){
            pt++;
        }

        if (*pt == '\0') {
            ereport(ERROR,
					(errmsg("invalid input syntax for type vector shape: \"%s\"", str_copy)));
        }

        shape[*shape_size] = strtof(pt, &end);

        (*shape_size)++;

        if(end == pt){
            ereport(ERROR,
					(errmsg("invalid input syntax for type vector shape: \"%s\"", str_copy)));
        }

        while(is_space(*end)){
            end++;
        }

        if (*end != '\0' && *end != '}'){
            ereport(ERROR,
					(errmsg("invalid input syntax for type vector shape: \"%s\"", str_copy)));
        }

        pt = strtok(NULL, ",");
    }

    if (end == NULL || *end != '}'){
        ereport(ERROR,
					(errmsg("malformed vector literal4: \"%s\"", str_copy)));
    }

    end++;

    while (is_space(*end)){
        end++;
    }

    for(pt = str_copy + 1; *pt != '\0'; pt++){
        if (pt[-1] == ',' && *pt == ','){
            ereport(ERROR,
					(errmsg("malformed vector literal5: \"%s\"", str_copy)));
        }
    }

    if(*shape_size < 1){
        ereport(ERROR,
					(errmsg("vector must have at least 1 dimension")));
    }

    pfree(str_copy);
}

static inline void 
parse_vector_str(char* str, unsigned int* dim, float* x,
                 unsigned int* shape_size, int32* shape)
{
    char* str_copy = pstrdup(str);
    char* index = str_copy;
    char* pt = NULL;
    char* end = NULL;

    while(is_space(*str)){
        str++;
    }

    if (*str != '['){
        ereport(ERROR,
				 errmsg("Vector contents must start with \"[\"."));
    }

    str++;
    pt = strtok(str, ",");
    end = pt;

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
					(errmsg("malformed vector literal3: \"%s\"", str_copy)));
    }

    end++;

    while (is_space(*end)){
        end++;
    }

    switch (*end) {
        case '{':
            while(*str_copy != '{'){
                str_copy++;
            }
            parse_vector_shape_str(str_copy, shape_size, shape);
            str_copy = index;
            break;
        case '\0':
            *shape_size = 1;
            shape[0] = (int32)(*dim);
            break;
        default:
            ereport(ERROR,
					(errmsg("malformed vector literal1: \"%s\"", str_copy)));
    }

    for(pt = str_copy + 1; *pt != '\0'; pt++){
        if (pt[-1] == ',' && *pt == ','){
            ereport(ERROR,
					(errmsg("malformed vector literal2: \"%s\"", str_copy)));
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
    float           *x = palloc(sizeof(float) * MAX_VECTOR_DIM);
    int32           shape[MAX_VECTOR_SHAPE_SIZE];
    Vector*         vector = NULL;
    unsigned int    dim = 0;
    unsigned int    shape_size = 0;
    unsigned int    shape_dim = 1;
    
    str = PG_GETARG_CSTRING(0);


    parse_vector_str(str, &dim, x, &shape_size, shape);


    // if(PG_NARGS() == 1){
    //     shape_size = 1;
    //     shape[0] = dim;
    // }else{
    //     shape_str = PG_GETARG_CSTRING(0);
    //     ereport(INFO,
	// 				(errmsg("shape_str:%s", shape_str)));
    //     parse_vector_shape_str(shape_str, &shape_size, shape);
    // }

    // Verify that the multiplication of shape values equals dim
    for(int i=0; i<shape_size; i++){
        shape_dim *= shape[i];
    }

    if(dim != shape_dim){
        ereport(ERROR,
					(errmsg("the multiplication of shape values not equals, dim:%d, shape_dim:%d", dim, shape_dim)));
    }
    vector = new_vector(dim, shape_size);

    for(int i=0; i<dim; ++i){
        vector->x[i] = x[i];
    }

    for(int i=0; i<shape_size; ++i){
        vector->shape[i] = shape[i];
    }

    pfree(x);
    PG_RETURN_POINTER(vector);
}

Datum
vector_output(PG_FUNCTION_ARGS)
{
    Vector* vector = NULL;
    StringInfoData result;
    unsigned int dim = 0;
    unsigned int shape_size = 0;

    vector = PG_GETARG_VECTOR_P(0);

    dim = VECTOR_DIM(vector);

    shape_size = VECTOR_SHAPE_SIZE(vector);

    if(dim > MAX_VECTOR_DIM){
        ereport(ERROR,
					(errmsg("dim is larger than 102400000 dim!")));
    }

    if(shape_size > MAX_VECTOR_SHAPE_SIZE){
        ereport(ERROR,
					(errmsg("shape size is larger than 10!")));
    }

    initStringInfo(&result);
    appendStringInfoChar(&result, '[');
    if(dim > 10){
        for(int i=0; i<3; ++i){
            appendStringInfoString(&result, DatumGetCString(DirectFunctionCall1(float4out, Float4GetDatum(vector->x[i]))));
            appendStringInfoChar(&result, ',');
        }
        appendStringInfoString(&result, "....");
        appendStringInfoChar(&result, ',');
        for(int i=dim-3; i<dim; ++i){
            appendStringInfoString(&result, DatumGetCString(DirectFunctionCall1(float4out, Float4GetDatum(vector->x[i]))));
            if (i != dim - 1) {
                appendStringInfoChar(&result, ',');
            }
        }
    }else{
        for(int i=0; i<dim; ++i){
            appendStringInfoString(&result, DatumGetCString(DirectFunctionCall1(float4out, Float4GetDatum(vector->x[i]))));
            if (i != dim - 1) {
                appendStringInfoChar(&result, ',');
            }
        }
    }
    appendStringInfoChar(&result, ']');
    
    appendStringInfoChar(&result, '{');
    for(int i=0; i<shape_size; ++i){
        appendStringInfoString(&result, DatumGetCString(DirectFunctionCall1(int4out, Int32GetDatum(vector->shape[i]))));
        if (i != shape_size - 1) {
            appendStringInfoChar(&result, ',');
        }
    }
    appendStringInfoChar(&result, '}');

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
    Datum           *elems = NULL;
    bool            *nulls = NULL;
    //int16		    typlen = 0;
	//bool		    typbyval = 0;
	//char		    typalign = 0;


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
					(errmsg("unsupport %u type to vector!", array_type)));
        }
    }

    if(array_length > MAX_VECTOR_DIM){
        ereport(ERROR,
					(errmsg("vector cannot have more than %d dimensions", MAX_VECTOR_DIM)));
    }

    vector = new_vector(array_length, 1);

    vector->shape[0] = array_length;
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
    int32            shape[MAX_VECTOR_SHAPE_SIZE];
    float            *x = palloc(sizeof(float) * MAX_VECTOR_DIM);
    unsigned int     shape_size = 0;
    unsigned int     shape_dim = 1;

    str = TextDatumGetCString(PG_GETARG_DATUM(0));

    parse_vector_str(str, &dim, x, &shape_size, shape);

    for(int i=0; i<shape_size; i++){
        shape_dim *= shape[i];
    }

    if(dim != shape_dim){
        ereport(ERROR,
					(errmsg("the multiplication of shape values not equals, dim:%d, shape_dim:%d", dim, shape_dim)));
    }

    vector = new_vector(dim, shape_size);
    
    for(int i=0; i<dim; ++i){
        vector->x[i] = x[i];
    }

    for(int i=0; i<shape_size; ++i){
        vector->shape[i] = shape[i];
    }

    pfree(x);
    PG_RETURN_POINTER(vector);
}

Datum
get_vector_data(PG_FUNCTION_ARGS)
{
    Vector	        *vector = NULL;
    //float           x[MAX_VECTOR_DIM];
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
get_vector_shape(PG_FUNCTION_ARGS)
{
    Vector	        *vector = NULL;
    //float           x[MAX_VECTOR_DIM];
    ArrayType       *result = NULL;
    Datum           *elems = NULL; 
    unsigned int    shape_size = 0;
    
    vector = PG_GETARG_VECTOR_P(0);

    shape_size = vector->shape_size;

    elems = (Datum *)palloc(sizeof(Datum *) * shape_size);

    for(int i=0; i<shape_size; ++i){
        elems[i] = Int32GetDatum(vector->shape[i]);
    }

    result = construct_array(elems, shape_size, INT4OID, sizeof(int32), true, 'i');

    pfree(elems);
    PG_RETURN_ARRAYTYPE_P(result);
}

Datum
vector_add(PG_FUNCTION_ARGS)
{
    Vector	        *vector_left = NULL;
    Vector          *vector_right = NULL;
    unsigned int    dim = 0;
    unsigned int    shape_size = 0;
    Vector          *result = NULL;

    vector_left = PG_GETARG_VECTOR_P(0);
    vector_right = PG_GETARG_VECTOR_P(1);


    if(vector_left->dim != vector_right->dim){
        ereport(ERROR,
					(errmsg("the two vectors have different dimensions!")));
    }

    if(!shape_equal(vector_left, vector_right)){
        ereport(ERROR,
					(errmsg("the two vectors have different shape!")));
    }

    dim = vector_left->dim;
    shape_size = vector_left->shape_size;

    result = new_vector(dim, shape_size);

    for(int i=0; i<dim; ++i){
        result->x[i] = vector_left->x[i] + vector_right->x[i];
        if(isinf(result->x[i])){
            ereport(ERROR,
					(errmsg("value out of range: overflow!")));
        }
    }

    for(int i=0; i<result->shape_size; ++i){
        result->shape[i] = vector_left->shape[i];
    }

    PG_RETURN_POINTER(result);
}

Datum
vector_sub(PG_FUNCTION_ARGS)
{
    Vector	        *vector_left = NULL;
    Vector          *vector_right = NULL;
    unsigned int    dim = 0;
    unsigned int    shape_size = 0;
    Vector          *result = NULL;

    vector_left = PG_GETARG_VECTOR_P(0);
    vector_right = PG_GETARG_VECTOR_P(1);


    if(vector_left->dim != vector_right->dim){
        ereport(ERROR,
					(errmsg("the two vectors have different dimensions!")));
    }

    if(!shape_equal(vector_left, vector_right)){
        ereport(ERROR,
					(errmsg("the two vectors have different shape!")));
    }

    dim = vector_left->dim;
    shape_size = vector_left->shape_size;

    result = new_vector(dim, shape_size);

    for(int i=0; i<dim; ++i){
        result->x[i] = vector_left->x[i] - vector_right->x[i];
        if(isinf(result->x[i])){
            ereport(ERROR,
					(errmsg("value out of range: overflow!")));
        }
    }

    for(int i=0; i<result->shape_size; ++i){
        result->shape[i] = vector_left->shape[i];
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

    if(!shape_equal(vector_left, vector_right)){
        PG_RETURN_BOOL(false);
    }

    for(int i=0; i<vector_left->dim; ++i){
        if(vector_left->x[i] != vector_right->x[i]){
            PG_RETURN_BOOL(false);
        }
    }

    PG_RETURN_BOOL(true);
}