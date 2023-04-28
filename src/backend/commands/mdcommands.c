#include "postgres.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


#include "access/heapam.h"
#include "access/htup_details.h"
#include "catalog/indexing.h"
#include "catalog/pg_model_info.h"
#include "commands/mdcommands.h"
#include "model/libtorch_wrapper.h"
#include "utils/builtins.h"
#include "utils/syscache.h"
#include "utils/timestamp.h"



/*
 *
 * craetemd 
 * 现在是只往系统表中添加记录，并不load
 *
 */
void
createmd(ParseState *pstate, const CreatemdStmt *stmt)
{
    char        *mdname = stmt->mdname;
    char        *mdpath = stmt->modelPath;
    char        *query  = pstate->p_sourcetext;
    Datum		new_record[Natts_pg_model_info];
	bool		new_record_nulls[Natts_pg_model_info];
    HeapTuple	tuple;
    Relation	pg_model_info_rel;

    // 先判断是否已经存在同名的model
    if(SearchSysCacheExists1(MODELNAME, CStringGetDatum(mdname))){
        ereport(ERROR,
				(errcode(ERRCODE_DUPLICATE_MODEL),
				 errmsg("model \"%s\" already exists", mdname)));
    }

    // 插入新model的记录

    pg_model_info_rel = table_open(ModelInfoRelationId, RowExclusiveLock);

    MemSet(new_record, 0, sizeof(new_record));
	MemSet(new_record_nulls, false, sizeof(new_record_nulls));

    new_record[Anum_pg_model_info_modelname - 1] = CStringGetDatum(mdname);
    new_record[Anum_pg_model_info_createtime - 1] = TimestampGetDatum(GetSQLLocalTimestamp(-1));
    new_record[Anum_pg_model_info_modelpath - 1] = CStringGetTextDatum(mdpath); // text宏别用错了
    new_record[Anum_pg_model_info_query - 1] = CStringGetTextDatum(query);
    
    new_record_nulls[Anum_pg_model_info_updatetime - 1] = true;
    tuple = heap_form_tuple(RelationGetDescr(pg_model_info_rel), new_record, new_record_nulls);

    CatalogTupleInsert(pg_model_info_rel, tuple);
    table_close(pg_model_info_rel, RowExclusiveLock);

    // ForceSyncCommit();
}


/*
 *  系统表中删除记录 
 */
void 
dropmd(ParseState *pstate, const DropmdStmt *stmt){
    // 判断是否存在该model
    char        *mdname = stmt->mdname;
    Relation	pg_model_info_rel;
    HeapTuple	tuple;
     
    if(!SearchSysCacheExists1(MODELNAME, CStringGetDatum(mdname))){
        ereport(ERROR,
				(errcode(ERRCODE_UNDEFINED_MODEL),
				 errmsg("model \"%s\" does not exist", mdname)));
    }


    // 删除model
    pg_model_info_rel = table_open(ModelInfoRelationId, RowExclusiveLock);
    tuple = SearchSysCache1(MODELNAME,CStringGetDatum(mdname));

    CatalogTupleDelete(pg_model_info_rel,&tuple->t_self);

    ReleaseSysCache(tuple);
    table_close(pg_model_info_rel, NoLock);

}

void
updatemd(ParseState *pstate, const UpdatemdStmt *stmt)
{
    HeapTuple	tuple;
    HeapTuple	newtuple;
	bool		nulls[Natts_pg_model_info];
	bool		replaces[Natts_pg_model_info];
	Datum		values[Natts_pg_model_info];
    char        *mdname = stmt->mdname;
    char        *mdpath = stmt->modelPath;
    char        *query  = pstate->p_sourcetext;
    Relation	pg_model_desc;

    // 只需要更新path
    replaces[Anum_pg_model_info_modelpath - 1] = true;
    values[Anum_pg_model_info_modelpath - 1] = CStringGetTextDatum(mdpath);
    nulls[Anum_pg_model_info_modelpath - 1] = false;

    replaces[Anum_pg_model_info_updatetime - 1] = true;
    values[Anum_pg_model_info_updatetime - 1] = TimestampGetDatum(GetSQLLocalTimestamp(-1));
    nulls[Anum_pg_model_info_updatetime - 1] = false;

    replaces[Anum_pg_model_info_query - 1] = true;
    values[Anum_pg_model_info_query - 1] = CStringGetTextDatum(query);
    nulls[Anum_pg_model_info_query - 1] = false;



    // 查原来的tuple
    pg_model_desc = table_open(ModelInfoRelationId, RowExclusiveLock);
    tuple = SearchSysCache1(MODELNAME, CStringGetDatum(mdname));
    if(!HeapTupleIsValid(tuple)) {
        ereport(ERROR,
				(errcode(ERRCODE_UNDEFINED_MODEL),
				 errmsg("model \"%s\" does not exist", mdname)));
    }

    // 更新tuple内容
    newtuple = heap_modify_tuple(tuple, RelationGetDescr(pg_model_desc), values, nulls, replaces);

    // 更新表内tuple
    CatalogTupleUpdate(pg_model_desc, &newtuple->t_self, newtuple);

    ReleaseSysCache(tuple);
    table_close(pg_model_desc,NoLock);


}