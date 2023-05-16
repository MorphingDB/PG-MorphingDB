#ifndef MODEL_H
#define MODEL_H

#include "libpq-fe.h"

extern char *parse_upload_model_path(const char *args);

extern Oid do_upload(char* file_path, bool *own_transaction);

extern char *reassemble_query(const char* query, Oid foid, const char* md5);
extern void do_upload_finish(int status, bool own_transaction);

extern char *get_file_md5(const char *file_path); 

#endif