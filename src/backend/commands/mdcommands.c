#include "postgres.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


#include "miscadmin.h"
#include "access/heapam.h"
#include "access/htup_details.h"
#include "access/genam.h"
#include "catalog/indexing.h"
#include "catalog/pg_model_info.h"
#include "commands/mdcommands.h"
#include "common/model_md5.h"
#include "common/relpath.h"
#include "libpq/libpq-fs.h"
#include "model/libtorch_wrapper.h"
#include "utils/acl.h"
#include "utils/builtins.h"
#include "utils/fmgroids.h"
#include "utils/rel.h"
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
    int16       version = get_model_max_version(stmt->mdname)+1;
    char *dbDir = GetDatabasePath(MyDatabaseId, MyDatabaseTableSpace);
    char *filename = psprintf("%s/%s/%s-%d",DataDir, dbDir, stmt->mdname, version );
    export_large_object(stmt->looid, filename, stmt->md5);

    Datum		new_record[Natts_pg_model_info];
	bool		new_record_nulls[Natts_pg_model_info];
    HeapTuple	tuple;
    Relation	pg_model_info_rel;

    char        *mdname = stmt->mdname;
    
    bool        active  = false;
    char        *md5 = stmt->md5;
    char        *user = GetUserNameFromId(GetUserId(), false);

    // // 先判断是否已经存在同名的model
    // if(SearchSysCacheExists1(MODELNAME, CStringGetDatum(mdname))){
    //     ereport(ERROR,
	// 			(errcode(ERRCODE_DUPLICATE_MODEL),
	// 			 errmsg("model \"%s\" already exists", mdname)));
    // }

    // 插入新model的记录
    pg_model_info_rel = table_open(ModelInfoRelationId, RowExclusiveLock);

    MemSet(new_record, 0, sizeof(new_record));
	MemSet(new_record_nulls, false, sizeof(new_record_nulls));

    new_record[Anum_pg_model_info_modelname - 1] = CStringGetDatum(mdname);
    new_record[Anum_pg_model_info_uploadtime- 1] = TimestampGetDatum(GetSQLLocalTimestamp(-1));
    new_record[Anum_pg_model_info_active - 1] = BoolGetDatum(active);
    new_record[Anum_pg_model_info_version - 1] = Int16GetDatum(version);
    new_record[Anum_pg_model_info_uploadby -1 ] =  CStringGetDatum(user);
    new_record[Anum_pg_model_info_md5 - 1] = CStringGetDatum(md5);
    new_record[Anum_pg_model_info_modelpath - 1] = CStringGetTextDatum(filename); // text宏别用错了
    // new_record[Anum_pg_model_info_query - 1] = CStringGetTextDatum(query);
    
    //new_record_nulls[Anum_pg_model_info_modelpath - 1] = true;
    new_record_nulls[Anum_pg_model_info_description - 1] = true;
    tuple = heap_form_tuple(RelationGetDescr(pg_model_info_rel), new_record, new_record_nulls);

    CatalogTupleInsert(pg_model_info_rel, tuple);
    table_close(pg_model_info_rel, RowExclusiveLock);

    ForceSyncCommit();
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
export_large_object(Oid lobjId, const char *filename, const char* fmd5)
{
    int32       lobj_fd;
    char        buf[8192];
    int         nbytes;
    int         dst_fd;
    char        hexsum[33]; // 摘要校验
    char        *err_msg;
    unsigned char md5_value[MD5_SIZE];
    MD5_CTX md5;
    
    
    lobj_fd = inv_open(lobjId, INV_READ, CurrentMemoryContext);
    if (lobj_fd < 0)
        ereport(ERROR, (errmsg("could not open large object %u", lobjId)));

    dst_fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC | PG_BINARY, 0666);
    if (dst_fd < 0){
        inv_close(lobj_fd);
        ereport(ERROR, (errcode_for_file_access(),
                        errmsg("could not create file \"%s\" ", filename)));
    }
    MD5Init(&md5);

    for (;;)
    {
        nbytes = inv_read(lobj_fd, buf, sizeof(buf));
        if (nbytes <= 0)
            break;
        // if(!pg_md5_hash(buf, nbytes, hexsum)) {
        //     err_msg = "model file check md5 failed";
        //     goto error;
        // }
        MD5Update(&md5, buf, nbytes);
        if ((int) write(dst_fd, buf, nbytes) != nbytes){
            inv_close(lobj_fd);
            close(dst_fd);
            remove(filename);
            ereport(ERROR, (errcode_for_file_access(),
                                    errmsg("err_msg")));
        }
    }

    inv_close(lobj_fd);
    if(close(dst_fd) != 0 ){
        // ereport(ERROR, (errcode_for_file_access(),
        //                 errmsg("could not close file \"%s\" ", filename)));
        remove(filename);
        ereport(ERROR, (errcode_for_file_access(),
                                    errmsg("could not close file")));
    }
    inv_drop(lobjId);
    MD5Final(&md5, md5_value);
    char *md5_str = palloc0(33);

    for(int i = 0; i < MD5_SIZE; i++)
	{
		snprintf(md5_str + i*2, 2+1, "%02x", md5_value[i]);
	}

    if(pg_strcasecmp(md5_str, fmd5)){
        remove(filename);
        ereport(ERROR, (errcode_for_file_access(),
                        errmsg("file md5 not match")));
    }

        
    return ;
}


/**
 * 
 * 查询已有的模型的版本最大值
 * 没有就是0
 * 
*/
int16
get_model_max_version(const char* model_name) {
    IndexScanDesc index_scan;
    ScanKeyData scan_key_data;
    TupleTableSlot *slot;
    ItemPointer tid;
    int16 max_version = 0;
    Relation modelinfo_relation;
    Relation index_relation;

    modelinfo_relation = table_open(ModelInfoRelationId, AccessShareLock);
    index_relation = index_open(ModelInfoNameVersionIndex,AccessShareLock);


    TupleDesc tupleDesc = RelationGetDescr(modelinfo_relation);
    slot = MakeSingleTupleTableSlot(tupleDesc, &TTSOpsVirtual);

    // 初始化一个scan key
    ScanKeyInit(&scan_key_data,
                Anum_pg_model_info_modelname,
                BTEqualStrategyNumber, F_NAMEEQ,
                CStringGetDatum(model_name));

    // 根据索引进行scan
    index_scan = index_beginscan(modelinfo_relation, 
                                 index_relation, 
                                 NULL, 
                                 1, 
                                 0);
    index_rescan(index_scan, &scan_key_data, 1, NULL, 0);

     // 循环遍历获取结果
    while ((tid = index_getnext_tid(index_scan, ForwardScanDirection)) != NULL){
        if (index_fetch_heap(index_scan, slot)){
            Datum datum;
            bool isNull;
            datum = slot_getattr(slot, Anum_pg_model_info_version, &isNull);

            if (!isNull){
                int16 version = DatumGetInt16(datum);
                if (version > max_version)
                {
                    max_version = version;
                }
            }
        }
        
    }
    ExecDropSingleTupleTableSlot(slot);

    index_endscan(index_scan);
    table_close(modelinfo_relation, AccessShareLock);
    index_close(index_relation,AccessShareLock);

    return max_version;
}