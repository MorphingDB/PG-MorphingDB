#include "catalog/model_info_d.h"
#include "postgres.h"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>


#include "miscadmin.h"
#include "access/heapam.h"
#include "access/htup_details.h"
#include "access/genam.h"
#include "catalog/indexing.h"
#include "catalog/model_info.h"
#include "catalog/model_layer_info.h"
#include "catalog/base_model_info.h"
#include "commands/mdcommands.h"
#include "common/model_md5.h"
#include "common/relpath.h"
#include "libpq/libpq-fs.h"
#include "model/libtorch_wrapper.h"
#include "model/predict_wrapper.h"
#include "storage/lockdefs.h"
#include "utils/acl.h"
#include "utils/builtins.h"
#include "utils/elog.h"
#include "utils/fmgroids.h"
#include "utils/rel.h"
#include "utils/syscache.h"
#include "utils/catcache.h"
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
    // int16       version = get_model_max_version(stmt->mdname)+1;
    char *dbDir = GetDatabasePath(MyDatabaseId, MyDatabaseTableSpace);
    char *filename = psprintf("%s/%s/%s",DataDir, dbDir, stmt->mdname);
    export_large_object(stmt->looid, filename, stmt->md5);

    Datum		new_record[Natts_model_info];
	bool		new_record_nulls[Natts_model_info];
    Datum		new_layer_record[Natts_model_layer_info];
	bool		new_layer_record_nulls[Natts_model_layer_info];

    HeapTuple	tuple;
    Relation	pg_model_info_rel;
    Relation    pg_model_layer_info_rel;

    char        *mdname = stmt->mdname;
    char        *desc   = stmt->desc;
    char        *md5 = stmt->md5;
    char        *base_model = stmt->base_model;
    char        *user = GetUserNameFromId(GetUserId(), false);

    ModelLayer  *model_layer = NULL;
    uint32       layer_size = 0;


    // 先判断是否已经存在同名的model
    if(SearchSysCacheExists1(MODELNAME, CStringGetDatum(mdname))){
        ereport(ERROR,
				(errcode(ERRCODE_DUPLICATE_MODEL),
				 errmsg("model \"%s\" already exists in model_info", mdname)));
    }

    if(SearchSysCacheExists1(LAYERMODELNAME, CStringGetDatum(mdname))){
        ereport(ERROR,
				(errcode(ERRCODE_DUPLICATE_MODEL),
				 errmsg("model \"%s\" already exists in model_layer_info", mdname)));
    }

    if(base_model != NULL){
        if(strcmp(mdname, base_model) == 0){
            ereport(ERROR,
                    (errcode(ERRCODE_DUPLICATE_MODEL),
                    errmsg("model name \"%s\" equals to base model", mdname)));
        }
        if(!SearchSysCacheExists1(BASEMODEL, CStringGetDatum(base_model))){
            ereport(ERROR,
                    (errcode(ERRCODE_UNDEFINED_BASE_MODEL),
                    errmsg("base model \"%s\" not exists in base_model_info", base_model)));
        }
    }



    // 插入新model的记录
    pg_model_info_rel = table_open(ModelInfoRelationId, RowExclusiveLock);

    MemSet(new_record, 0, sizeof(new_record));
	MemSet(new_record_nulls, false, sizeof(new_record_nulls));

    new_record[Anum_model_info_modelname - 1] = CStringGetDatum(mdname);
    new_record[Anum_model_info_createtime- 1] = TimestampGetDatum(GetSQLLocalTimestamp(-1));
    new_record[Anum_model_info_uploadby -1 ]  =  CStringGetDatum(user);
    //new_record[Anum_model_info_md5 - 1]       = CStringGetDatum(md5);
    //new_record[Anum_model_info_modelpath - 1] = CStringGetTextDatum(filename); // text宏别用错了

    if (base_model == NULL) {
        /* 
           new model need save md5 in model_info, 
           base model in model_info is empty,
           save model path in model_info
        */
        new_record_nulls[Anum_model_info_basemodel - 1] = true;
        new_record[Anum_model_info_md5 - 1] = CStringGetDatum(md5);
        new_record[Anum_model_info_modelpath - 1] = CStringGetTextDatum(filename);
    }else {
         /* 
           have base model md5 is empty in model_info, 
           base model in model_info is not empty,
           model path in model_info is empty
        */
        new_record_nulls[Anum_model_info_md5 - 1] = true;
        new_record[Anum_model_info_basemodel - 1] = CStringGetDatum(base_model);
        new_record_nulls[Anum_model_info_modelpath - 1] = true;
    }

    if(desc == NULL) {
        new_record_nulls[Anum_model_info_description - 1] = true;    
    }else {
        new_record[Anum_model_info_description - 1] = CStringGetTextDatum(desc);
    }

    //new_record[Anum_model_info_updatetime- 1] = TimestampGetDatum(GetSQLLocalTimestamp(-1));
    tuple = heap_form_tuple(RelationGetDescr(pg_model_info_rel), new_record, new_record_nulls);

    CatalogTupleInsert(pg_model_info_rel, tuple);
    table_close(pg_model_info_rel, RowExclusiveLock);

    if(base_model != NULL){
        // insert model parameter
        model_parameter_extraction(filename, &model_layer, &layer_size);
        if(model_layer == NULL){
            ereport(ERROR,
                    errmsg("model layer empty"));
        }

        pg_model_layer_info_rel = table_open(ModelLayerInfoRelationId, RowExclusiveLock);
        

        for(int16 i=1; i<=layer_size; ++i){
            MemSet(new_layer_record, 0, sizeof(new_layer_record));
            MemSet(new_layer_record_nulls, false, sizeof(new_layer_record_nulls));
            new_layer_record[Anum_model_layer_info_layermodelname-1] = CStringGetDatum(mdname);
            new_layer_record[Anum_model_layer_info_layername-1] = CStringGetTextDatum(model_layer[i-1].layer_name);
            new_layer_record[Anum_model_layer_info_layerindex-1] = Int16GetDatum(i);
            new_layer_record[Anum_model_layer_info_parameter-1] = PointerGetDatum(model_layer[i-1].layer_parameter);
            tuple = heap_form_tuple(RelationGetDescr(pg_model_layer_info_rel), new_layer_record, new_layer_record_nulls);
            CatalogTupleInsert(pg_model_layer_info_rel, tuple);
        }
        table_close(pg_model_layer_info_rel, RowExclusiveLock);
    }

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
    Relation    pg_model_layer_info_rel;
    HeapTuple	tuple;
    Datum       oldfilenamedatum;
    char        *oldFilename;
    bool        isnull;
     
    if(!SearchSysCacheExists1(MODELNAME, CStringGetDatum(mdname))){
        ereport(ERROR,
				(errcode(ERRCODE_UNDEFINED_MODEL),
				 errmsg("model \"%s\" does not exist in model_info", mdname)));
    }

    // delete model_info 
    pg_model_info_rel = table_open(ModelInfoRelationId, RowExclusiveLock);
    tuple = SearchSysCache1(MODELNAME,CStringGetDatum(mdname));

    oldfilenamedatum = SysCacheGetAttr(MODELNAME, tuple, Anum_model_info_modelpath, &isnull);
    // has base model
    if(isnull){
        oldFilename = TextDatumGetCString(oldfilenamedatum);
        // delete model file
        remove(oldFilename);
    }

    CatalogTupleDelete(pg_model_info_rel,&tuple->t_self);

    ReleaseSysCache(tuple);
    table_close(pg_model_info_rel, NoLock);

    // has base model
    if(isnull){
        if(!SearchSysCacheExists1(LAYERMODELNAME, CStringGetDatum(mdname))){
            ereport(ERROR,
                    (errcode(ERRCODE_UNDEFINED_MODEL),
                    errmsg("model \"%s\" does not exist in model_layer_inf1o", mdname)));
        }
        // delete model_layer_info
        pg_model_layer_info_rel = table_open(ModelLayerInfoRelationId, RowExclusiveLock);
        if(SearchSysCacheExists1(LAYERMODELNAME, CStringGetDatum(mdname))){
            CatCList* parameter_list = SearchSysCacheList1(LAYERMODELNAME,CStringGetDatum(mdname));
            for(int i=0; i<parameter_list->n_members; i++){
                HeapTuple proctup = &parameter_list->members[i]->tuple;
                CatalogTupleDelete(pg_model_layer_info_rel,&proctup->t_self);
            }
            ReleaseCatCacheList(parameter_list);
        }
        table_close(pg_model_layer_info_rel, NoLock);
    }
}

void
updatemd(ParseState *pstate, const UpdatemdStmt *stmt)
{
    char random_suffix[5];	
    generate_random_digits(random_suffix, 4);
    
    char *dbDir = GetDatabasePath(MyDatabaseId, MyDatabaseTableSpace);
    char *filename = psprintf("%s/%s/%s-%s",DataDir, dbDir, stmt->mdname,random_suffix);
    export_large_object(stmt->looid, filename, stmt->md5);

    HeapTuple	tuple;
    HeapTuple	newtuple;
	bool		nulls[Natts_model_info] = {true};
	bool		replaces[Natts_model_info] = {false};
    bool        isnull;
	Datum		values[Natts_model_info];
    char        *mdname = stmt->mdname;
    char        *desc   = stmt->desc;
    char        *md5 = stmt->md5;
    char        *user = GetUserNameFromId(GetUserId(), false);
    Datum       oldfilenamedatum;
    char        *oldFilename;

    Relation	pg_model_desc;

    replaces[Anum_model_info_modelpath - 1] = true;
    values[Anum_model_info_modelpath - 1] = CStringGetTextDatum(filename);
    nulls[Anum_model_info_modelpath - 1] = false;

    replaces[Anum_model_info_uploadby - 1] = true;
    values[Anum_model_info_uploadby - 1] = CStringGetDatum(user);
    nulls[Anum_model_info_uploadby - 1] = false;

    replaces[Anum_model_info_md5 - 1] = true;
    values[Anum_model_info_md5 - 1] = CStringGetDatum(md5);
    nulls[Anum_model_info_md5 - 1] = false;

    // replaces[Anum_model_info_updatetime - 1] = true;
    // values[Anum_model_info_updatetime - 1] = TimestampGetDatum(GetSQLLocalTimestamp(-1));
    // nulls[Anum_model_info_updatetime - 1] = false;

    if(desc != NULL) {
        replaces[Anum_model_info_description - 1] = true;
        values[Anum_model_info_description - 1] = CStringGetTextDatum(desc);
        nulls[Anum_model_info_description - 1] = false;
    }

    // 查原来的tuple
    pg_model_desc = table_open(ModelInfoRelationId, RowExclusiveLock);
    tuple = SearchSysCache1(MODELNAME, CStringGetDatum(mdname));
    if(!HeapTupleIsValid(tuple)) {
        ereport(ERROR,
				(errcode(ERRCODE_UNDEFINED_MODEL),
				 errmsg("model \"%s\" does not exist in model_info", mdname)));
    }
    


    oldfilenamedatum = SysCacheGetAttr(MODELNAME, tuple, Anum_model_info_modelpath, &isnull);
    
    oldFilename = TextDatumGetCString(oldfilenamedatum);

    // 更新tuple内容
    newtuple = heap_modify_tuple(tuple, RelationGetDescr(pg_model_desc), values, nulls, replaces);

    // 更新表内tuple
    CatalogTupleUpdate(pg_model_desc, &newtuple->t_self, newtuple);

    ReleaseSysCache(tuple);
    table_close(pg_model_desc,NoLock);

    // 删除原文件
    remove(oldFilename);

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
// int16
// get_model_max_version(const char* model_name) {
//     IndexScanDesc index_scan;
//     ScanKeyData scan_key_data;
//     TupleTableSlot *slot;
//     ItemPointer tid;
//     int16 max_version = 0;
//     Relation modelinfo_relation;
//     Relation index_relation;

//     modelinfo_relation = table_open(ModelInfoRelationId, AccessShareLock);
//     index_relation = index_open(ModelInfoNameVersionIndex,AccessShareLock);


//     TupleDesc tupleDesc = RelationGetDescr(modelinfo_relation);
//     slot = MakeSingleTupleTableSlot(tupleDesc, &TTSOpsVirtual);

//     // 初始化一个scan key
//     ScanKeyInit(&scan_key_data,
//                 Anum_pg_model_info_modelname,
//                 BTEqualStrategyNumber, F_NAMEEQ,
//                 CStringGetDatum(model_name));

//     // 根据索引进行scan
//     index_scan = index_beginscan(modelinfo_relation, 
//                                  index_relation, 
//                                  NULL, 
//                                  1, 
//                                  0);
//     index_rescan(index_scan, &scan_key_data, 1, NULL, 0);

//      // 循环遍历获取结果
//     while ((tid = index_getnext_tid(index_scan, ForwardScanDirection)) != NULL){
//         if (index_fetch_heap(index_scan, slot)){
//             Datum datum;
//             bool isNull;
//             datum = slot_getattr(slot, Anum_pg_model_info_version, &isNull);

//             if (!isNull){
//                 int16 version = DatumGetInt16(datum);
//                 if (version > max_version)
//                 {
//                     max_version = version;
//                 }
//             }
//         }
        
//     }
//     ExecDropSingleTupleTableSlot(slot);

//     index_endscan(index_scan);
//     table_close(modelinfo_relation, AccessShareLock);
//     index_close(index_relation,AccessShareLock);

//     return max_version;
// }


void generate_random_digits(char* buf, size_t len)
{
    unsigned char random_bytes[4];
    if (!pg_strong_random(random_bytes, sizeof(random_bytes)))
    {
        ereport(LOG,
                (errmsg("could not generate random bytes")));
        return STATUS_ERROR;
    }

    for (size_t i = 0; i < len; i++)
    {
        buf[i] = '0' + (random_bytes[i % sizeof(random_bytes)] % 10);
    }

    buf[len] = '\0';  // 确保字符串以 null 结束
}
