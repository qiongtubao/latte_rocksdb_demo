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
    printf("rocksdb open %s\n", dir);
    if (err != NULL) {
        printf("open db %s fail\n", dir);
        return NULL;
    }
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