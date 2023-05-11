#include "postgres_fe.h"
#include "model.h"


#include <signal.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>				/* for isatty */
#else
#include <io.h>					/* I think */
#endif

#include "libpq-fe.h"
#include "pqexpbuffer.h"

#include "settings.h"
#include "common.h"
#include "prompt.h"
#include "stringutils.h"

#include "common/logging.h"




/*
 * copy from  
 */

static bool
start_lo_xact(const char *operation, bool *own_transaction)
{
	PGTransactionStatusType tstatus;
	PGresult   *res;

	*own_transaction = false;

	if (!pset.db)
	{
		pg_log_error("%s: not connected to a database", operation);
		return false;
	}

	tstatus = PQtransactionStatus(pset.db);

	switch (tstatus)
	{
		case PQTRANS_IDLE:
			/* need to start our own xact */
			if (!(res = PSQLexec("BEGIN")))
				return false;
			PQclear(res);
			*own_transaction = true;
			break;
		case PQTRANS_INTRANS:
			/* use the existing xact */
			break;
		case PQTRANS_INERROR:
			pg_log_error("%s: current transaction is aborted", operation);
			return false;
		default:
			pg_log_error("%s: unknown transaction status", operation);
			return false;
	}

	return true;
}

static bool
finish_lo_xact(const char *operation, bool own_transaction)
{
	PGresult   *res;

	if (own_transaction && pset.autocommit)
	{
		/* close out our own xact */
		if (!(res = PSQLexec("COMMIT")))
		{
			res = PSQLexec("ROLLBACK");
			PQclear(res);
			return false;
		}
		PQclear(res);
	}

	return true;
}


static bool
fail_lo_xact(const char *operation, bool own_transaction)
{
	PGresult   *res;

	if (own_transaction && pset.autocommit)
	{
		/* close out our own xact */
		res = PSQLexec("ROLLBACK");
		PQclear(res);
	}

	return false;				/* always */
}



/**
 * 
 *  create model model_name path 'file_path' ;
 *  返回文件名
 *  
 */
char * 
parse_upload_model_path(const char *args)
{
    char	   *token;
	const char *whitespace = " \t\n\r";
	// char		nonstd_backslash = standard_strings() ? 0 : '\\';
	
    if (!args)
	{
		pg_log_error("model arguments required");
		return NULL;
	}

	/* Split the SQL query into tokens */
	token = strtokx(args, whitespace, NULL, "'", 0, false, false,pset.encoding);
	while (token != NULL && pg_strcasecmp(token, "path") != 0)
	{
		token = strtokx(NULL, whitespace, NULL, "'", 0, false, false,pset.encoding);
	}

	if(token == NULL || strlen(token) == 0)
		goto error;

	// 否则当前是path，解析后面的路径
	token = strtokx(NULL, whitespace, NULL, "'", 0, false, true ,pset.encoding);
	
	
	if(token == NULL || strlen(token) == 0 || strcmp(token, ";") == 0)
		goto error;

	printf("token: %s\n",token);	
	return token;

error:
	if (token)
		pg_log_error("model can't get file path at \"%s\"", token);
	else
		pg_log_error("model can't get file path at end of line");

	return NULL;
}

/**
 * 
 * 上传文件，并且返回大对象的OID
 * 
*/
Oid do_upload(char* file_path){

	expand_tilde(&file_path);


	PGresult   *res;
	Oid			loid;
	char		oidbuf[32];
	bool		own_transaction;

	if (!start_lo_xact("\\do_upload", &own_transaction))
		goto error;

	SetCancelConn();
	loid = lo_import(pset.db, file_path);
	ResetCancelConn();

	if (loid == InvalidOid)
	{
		pg_log_info("%s", PQerrorMessage(pset.db));
		fail_lo_xact("\\do_upload", own_transaction);
		goto error;
	}

	if (!finish_lo_xact("\\do_upload", own_transaction))
		goto error;


	sprintf(oidbuf, "%u", loid);
	SetVariable(pset.vars, "LASTOID", oidbuf);

	return loid;

error:
	return InvalidOid;

}


/**
 * 
 * 把path后面的文件路径替换成oid
 * 
 * 
*/
char *reassemble_query(const char* query, Oid foid){

	char	   *token;
	const char *whitespace = " \t\n\r";
	PQExpBufferData buf;
	
	initPQExpBuffer(&buf);

	token = strtokx(query, whitespace, NULL, "'", 0, false, false,pset.encoding);
	while( pg_strcasecmp(token, "path") != 0 ) {
		// printf("token: %s\n", token);
		appendPQExpBuffer(&buf, "%s ",token);
		token = strtokx(NULL, whitespace, NULL, "'", 0, false, false,pset.encoding);
	}

	appendPQExpBufferStr(&buf, "path ");
	appendPQExpBuffer(&buf, "%u ;", foid);
	char *res = pg_strdup(buf.data);
	termPQExpBuffer(&buf);
	return res;
}

