/*-------------------------------------------------------------------------
 *
 * base_model_info.h
 *-------------------------------------------------------------------------
 */
#ifndef BASE_MODEL_INFO_H
#define BASE_MODEL_INFO_H

#include "catalog/genbki.h"
#include "catalog/base_model_info_d.h"


CATALOG(base_model_info,2228,BaseModelInfoRelationId) 
{
	
	/*base model name */
	NameData	basemodel;
#ifdef CATALOG_VARLEN	
	NameData 	md5;				
	text    	modelpath;
	/*  注意：注释只能够是这种格式  */
#endif
    
} FormData_pg_base_model_layer_info;
//



typedef FormData_pg_base_model_layer_info *Form_pg_base_model_info;

#endif 