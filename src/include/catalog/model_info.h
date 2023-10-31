/*-------------------------------------------------------------------------
 *
 * pg_model_info.h
 *-------------------------------------------------------------------------
 */
#ifndef PG_MODEL_INFO_H
#define PG_MODEL_INFO_H

#include "catalog/genbki.h"
#include "catalog/model_info_d.h"


CATALOG(model_info,2023,ModelInfoRelationId) 
{
	
	/* model name */
	NameData	modelname;
#ifdef CATALOG_VARLEN	
	timestamp   createtime;
	NameData    uploadby;
	NameData 	md5;				
	text    	modelpath;
	text 		description;
	timestamp   updatetime;
	/*  注意：注释只能够是这种格式  */
#endif
    
} FormData_pg_model_info;



typedef FormData_pg_model_info *Form_pg_model_info;

#endif 