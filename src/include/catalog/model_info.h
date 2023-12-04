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
	timestamp   createtime BKI_FORCE_NULL BKI_DEFAULT(_null_);
	NameData    uploadby BKI_FORCE_NULL BKI_DEFAULT(_null_);
	NameData 	md5 BKI_FORCE_NULL BKI_DEFAULT(_null_);
	text    	modelpath;
	NameData    basemodel BKI_FORCE_NULL BKI_DEFAULT(_null_);
	text 		description BKI_FORCE_NULL BKI_DEFAULT(_null_);
	/*  注意：注释只能够是这种格式  */
#endif
    
} FormData_pg_model_info;



typedef FormData_pg_model_info *Form_pg_model_info;

#endif 