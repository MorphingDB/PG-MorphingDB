#include "postgres.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


#include "miscadmin.h"
#include "access/heapam.h"
#include "access/htup_details.h"
#include "catalog/indexing.h"
#include "catalog/pg_model_info.h"
#include "commands/mdcommands.h"
#include "common/md5.h"
#include "common/relpath.h"
#include "libpq/libpq-fs.h"
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
    char *dbDir = GetDatabasePath(MyDatabaseId, MyDatabaseTableSpace);
    char *filename = psprintf("%s/%s/%s",DataDir, dbDir,"your_file_name" );

    export_large_object(stmt->looid, filename, stmt->md5);
    // char        *mdname = stmt->mdname;
    // char        *mdpath = stmt->modelPath;
    // char        *query  = pstate->p_sourcetext;
    // Datum		new_record[Natts_pg_model_info];
	// bool		new_record_nulls[Natts_pg_model_info];
    // HeapTuple	tuple;
    // Relation	pg_model_info_rel;

    // // 先判断是否已经存在同名的model
    // if(SearchSysCacheExists1(MODELNAME, CStringGetDatum(mdname))){
    //     ereport(ERROR,
	// 			(errcode(ERRCODE_DUPLICATE_MODEL),
	// 			 errmsg("model \"%s\" already exists", mdname)));
    // }

    // // 插入新model的记录

    // pg_model_info_rel = table_open(ModelInfoRelationId, RowExclusiveLock);

    // MemSet(new_record, 0, sizeof(new_record));
	// MemSet(new_record_nulls, false, sizeof(new_record_nulls));

    // new_record[Anum_pg_model_info_modelname - 1] = CStringGetDatum(mdname);
    // new_record[Anum_pg_model_info_createtime - 1] = TimestampGetDatum(GetSQLLocalTimestamp(-1));
    // new_record[Anum_pg_model_info_modelpath - 1] = CStringGetTextDatum(mdpath); // text宏别用错了
    // new_record[Anum_pg_model_info_query - 1] = CStringGetTextDatum(query);
    
    // new_record_nulls[Anum_pg_model_info_updatetime - 1] = true;
    // tuple = heap_form_tuple(RelationGetDescr(pg_model_info_rel), new_record, new_record_nulls);

    // CatalogTupleInsert(pg_model_info_rel, tuple);
    // table_close(pg_model_info_rel, RowExclusiveLock);

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
     
    // if(!SearchSysCacheExists1(MODELNAME, CStringGetDatum(mdname))){
    //     ereport(ERROR,
	// 			(errcode(ERRCODE_UNDEFINED_MODEL),
	// 			 errmsg("model \"%s\" does not exist", mdname)));
    // }


    // // 删除model
    // pg_model_info_rel = table_open(ModelInfoRelationId, RowExclusiveLock);
    // tuple = SearchSysCache1(MODELNAME,CStringGetDatum(mdname));

    // CatalogTupleDelete(pg_model_info_rel,&tuple->t_self);

    // ReleaseSysCache(tuple);
    // table_close(pg_model_info_rel, NoLock);

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

    // replaces[Anum_pg_model_info_updatetime - 1] = true;
    // values[Anum_pg_model_info_updatetime - 1] = TimestampGetDatum(GetSQLLocalTimestamp(-1));
    // nulls[Anum_pg_model_info_updatetime - 1] = false;

    // replaces[Anum_pg_model_info_query - 1] = true;
    // values[Anum_pg_model_info_query - 1] = CStringGetTextDatum(query);
    // nulls[Anum_pg_model_info_query - 1] = false;



    // // 查原来的tuple
    // pg_model_desc = table_open(ModelInfoRelationId, RowExclusiveLock);
    // tuple = SearchSysCache1(MODELNAME, CStringGetDatum(mdname));
    // if(!HeapTupleIsValid(tuple)) {
    //     ereport(ERROR,
	// 			(errcode(ERRCODE_UNDEFINED_MODEL),
	// 			 errmsg("model \"%s\" does not exist", mdname)));
    // }

    // // 更新tuple内容
    // newtuple = heap_modify_tuple(tuple, RelationGetDescr(pg_model_desc), values, nulls, replaces);

    // // 更新表内tuple
    // CatalogTupleUpdate(pg_model_desc, &newtuple->t_self, newtuple);

    // ReleaseSysCache(tuple);
    // table_close(pg_model_desc,NoLock);


}




void
export_large_object(Oid lobjId, const char *filename, const char* md5)
{
    int32       lobj_fd;
    char        buf[8192];
    int         nbytes;
    int         dst_fd;
    char        hexsum[33]; // 摘要校验
    char        *err_msg;

    
    lobj_fd = inv_open(lobjId, INV_READ, CurrentMemoryContext);
    if (lobj_fd < 0)
        ereport(ERROR, (errmsg("could not open large object %u", lobjId)));

    dst_fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC | PG_BINARY, 0666);
    if (dst_fd < 0){
        inv_close(lobj_fd);
        ereport(ERROR, (errcode_for_file_access(),
                        errmsg("could not create file \"%s\" ", filename)));
    }

    for (;;)
    {
        nbytes = inv_read(lobj_fd, buf, sizeof(buf));
        if (nbytes <= 0)
            break;
        if(!pg_md5_hash(buf, nbytes, hexsum)) {
            err_msg = "model file check md5 failed";
            goto error;
        }
        if ((int) write(dst_fd, buf, nbytes) != nbytes){
            err_msg = "could not save model file";
            goto error;
        }
    }

    inv_close(lobj_fd);
    if(close(dst_fd) != 0 ){
        ereport(ERROR, (errcode_for_file_access(),
                        errmsg("could not close file \"%s\" ", filename)));
        goto error;
    }
    inv_drop(lobjId);



    return ;

error:
    inv_close(lobj_fd);
    close(dst_fd);
    remove(filename);
    ereport(ERROR, (errcode_for_file_access(),
                        errmsg(err_msg)));
}