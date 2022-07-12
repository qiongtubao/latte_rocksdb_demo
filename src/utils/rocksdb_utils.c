#include <stdlib.h>
#include <stdio.h>
#include "rocksdb_utils.h"


rocksdb_options_t* initDbOpts() {
    rocksdb_options_t* options = rocksdb_options_create();
    rocksdb_options_set_create_if_missing(options, 1); 
    rocksdb_options_enable_statistics(options);
    rocksdb_options_set_stats_dump_period_sec(options, 60);
    rocksdb_options_set_max_write_buffer_number(options, 6);
    rocksdb_options_set_max_bytes_for_level_base(options, 512*1024*1024);

    // rocksdb_options_set_disable_auto_compactions(options, 1);

    // rocksdb_options_set_max_open_files(db_opts, 10);
    // rocksdb_options_set_paranoid_checks(options, 1);

    // rocksdb_options_set_comparator(options, cmp);
    // rocksdb_options_set_error_if_exists(options, 1);
    // rocksdb_options_set_env(options, env);
    // rocksdb_options_set_info_log(options, NULL);
    // rocksdb_options_set_write_buffer_size(options, 100000);
    // rocksdb_options_set_paranoid_checks(options, 1);
    // rocksdb_options_set_max_open_files(options, 10);
    // rocksdb_options_set_base_background_compactions(options, 1);
    // rocksdb_options_set_compression(options, rocksdb_no_compression);
    // rocksdb_options_set_compression_options(options, -14, -1, 0, 0);
    return options;
}


rocksdb_readoptions_t* initReadOpts() {
    rocksdb_readoptions_t* ropts;
    ropts = rocksdb_readoptions_create();
    rocksdb_readoptions_set_verify_checksums(ropts, 0);
    rocksdb_readoptions_set_fill_cache(ropts, 0);
    return ropts;
}



rocksdb_writeoptions_t* initWriteOpts() {
    rocksdb_writeoptions_t* wopts;
    wopts = rocksdb_writeoptions_create();
    rocksdb_writeoptions_disable_WAL(wopts, 1);
    return wopts;
}

rocksdb_flushoptions_t* initFlushOpts() {
    rocksdb_flushoptions_t *fopts;
    fopts = rocksdb_flushoptions_create();
    rocksdb_flushoptions_set_wait(fopts, 1);
    return fopts;
}


rocksdb_t* openDb(rocksdb_options_t* options, char* dir) {
    char* err = NULL;
    rocksdb_t* db = rocksdb_open(options, dir, &err);
    if (err != NULL) {
        printf("open db %s fail\n", dir);
        return NULL;
    }
    printf("rocksdb open %s\n", dir);
    return db;
}



char* readDb(rocksdb_t* db, rocksdb_readoptions_t* ropts, char* key, int key_size, int* value_size) {
    char* err = NULL;
    char* value = rocksdb_get(db, ropts, key, key_size, value_size, &err);
    if (err != NULL) {
        printf("read key:%s fail\n", key);
        return NULL;
    }
    return value;
}


int flushDB(rocksdb_t* db, rocksdb_flushoptions_t* fopts) {
    char* err = NULL;
    rocksdb_flush(db, fopts, &err);
    if(err != NULL) {
        printf("flush fail\n");
        return 0;
    }
    return 1;
}

int writeDb(rocksdb_t* db, rocksdb_writeoptions_t* wopts, char* key, int key_size, char* value, int value_size) {
    char *err = NULL;
    rocksdb_put(db, wopts, key, key_size, value, value_size, &err);
    if (err != NULL) {
        printf("write db key:%s, value:%s, error: %s\n",key, value, err);
        return 0;
    }
    return 1;
}



rocksdb_checkpoint_t* createCheckpoint(rocksdb_t* db, char* dir, uint64_t log_size_for_flush) {
    char* err = NULL;
    rocksdb_checkpoint_t* checkpoint = rocksdb_checkpoint_object_create(db, &err);
    if (err != NULL) {
        printf("checkpoint create fail :%s\n", err);
        return NULL;
    }
    rocksdb_checkpoint_create(checkpoint, dir, log_size_for_flush, &err);
    return checkpoint;
}

latte_rocksdb_cf* createLatteRocksdbCf(size_t len) {
    latte_rocksdb_cf* cf = malloc(sizeof(latte_rocksdb_cf));
    cf->db = NULL;
    if (len != 0) {
        cf->handles = malloc(sizeof(rocksdb_column_family_handle_t*));
    } else {
        cf->handles = NULL;
    }
    cf->len = len;
    return cf;
}

void releaseRocksdbCf(latte_rocksdb_cf* cf) {
    if (cf->len > 0) {
        free(cf->handles);
    }
    free(cf);
}
latte_rocksdb_cf* openDbByColumnFamily(rocksdb_options_t* options, char* dir) {
    char* err = NULL;
    // rocksdb_column_family_handle_t* handles[2];
    latte_rocksdb_cf* cfs = createLatteRocksdbCf(1);
    const char* default_cf_name = "default";
    rocksdb_t* db = rocksdb_open_column_families(options, dir, 1, &default_cf_name, &options, cfs->handles, &err);
    if (err != NULL) {
        printf("open db %s fail\n", dir);
        return NULL;
    }
    printf("rocksdb open %s\n", dir);
    cfs->db = db;
    return cfs;
}

int writeDbCf(latte_rocksdb_cf* cfs, rocksdb_writeoptions_t* wopts, char* key, int key_size, char* value, int value_size) {
    char *err = NULL;
    rocksdb_put_cf(cfs->db, wopts, cfs->handles[0], key, key_size, value, value_size, &err);
    if (err != NULL) {
        printf("write db key:%s, value:%s, error: %s\n",key, value, err);
        return 0;
    }
    return 1;
}