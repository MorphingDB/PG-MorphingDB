/*-------------------------------------------------------------------------
 *
 * pg_model_info.h
 *-------------------------------------------------------------------------
 */
#ifndef PG_MODEL_INFO_H
#define PG_MODEL_INFO_H

#include "catalog/genbki.h"
#include "catalog/pg_model_info_d.h"


CATALOG(pg_model_info,2023,ModelInfoRelationId) 
{
	Oid			oid;			/* oid */

	/* model name */
	NameData	modelname;
    NameData    modelpath;

} FormData_pg_model_info;



typedef FormData_pg_model_info *Form_pg_model_info;

#endif 