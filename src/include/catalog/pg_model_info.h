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
	
	/* model name */
	NameData	modelname;
	
#ifdef CATALOG_VARLEN	
	timestamp   createtime;
	text    	modelpath;
	text 		query;
	timestamp   updatetime; /* nullable的字段需要放到复合类型下面 */
	/*  注意：注释只能够是这种格式  */
#endif
    
} FormData_pg_model_info;



typedef FormData_pg_model_info *Form_pg_model_info;

#endif 