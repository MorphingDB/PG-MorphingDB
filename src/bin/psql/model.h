#ifndef MODEL_H
#define MODEL_H

#include "libpq-fe.h"

extern char *parse_upload_model_path(const char *args);

extern Oid do_upload(char* file_path);

extern char *reassemble_query(const char* query, Oid foid);

#endif