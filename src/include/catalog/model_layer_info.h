/*-------------------------------------------------------------------------
 *
 * pg_model_layer_info.h
 *-------------------------------------------------------------------------
 */
#ifndef PG_MODEL_LAYER_INFO_H
#define PG_MODEL_LAYER_INFO_H


#include "catalog/genbki.h"
#include "catalog/model_layer_info_d.h"


CATALOG(model_layer_info,2121,ModelLayerInfoRelationId) 
{
	
	/* model name */
	NameData	layermodelname;
#ifdef CATALOG_VARLEN	
	NameData    layername;
	int16 	    layerindex;				
	Vector    	parameter;
	/*  注意：注释只能够是这种格式  */
#endif
    
} FormData_pg_model_layer_info;


typedef FormData_pg_model_layer_info *Form_pg_model_layer_info;

#endif