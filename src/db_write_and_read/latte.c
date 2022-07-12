//jemalloc
#include <rocksdb/c.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../utils/latte_utils.h"
#include "../utils/rocksdb_utils.h"

#include <unistd.h>
int encode(char* buf, char* pre, long long index) {
    int len = sprintf(buf, "%s-%lld", pre, index);
    buf[len] = '\0';
    return len;
}

void create_random_value(char* value, int size) {
    for(int i = 0 ; i < size; i++) {
        value[i] = rand() % 10000;
    }
}

//#define VALUE_SIZE (1024 * 1024)
#define VALUE_SIZE 3
const size_t KiB = 1024L;
const size_t MiB = 1024L * KiB;
const size_t GiB = 1024L * MiB;
int main() {

    rocksdb_options_t* db_opts;
    rocksdb_cache_t* block_cache;
    rocksdb_writeoptions_t* wopts;
    rocksdb_readoptions_t* ropts;
    rocksdb_flushoptions_t* fopts;
    
    struct rocksdb_block_based_table_options_t *block_opts ;
    // db_opts = rocksdb_options_create();
    // rocksdb_options_set_create_if_missing(db_opts, 1); 
    // rocksdb_options_set_stats_dump_period_sec(db_opts, 60);
    // rocksdb_options_set_max_write_buffer_number(db_opts, 6);
    // rocksdb_options_set_max_bytes_for_level_base(db_opts, 512*1024*1024);
    db_opts = initDbOpts();
    // rocksdb_options_set_compaction_style(db_opts, rocksdb_level_compaction);
    // rocksdb_options_set_max_open_files(db_opts, -1);
    // rocksdb_options_set_max_subcompactions(db_opts, 1);
    // rocksdb_options_set_max_background_flushes(db_opts, 2);
    // rocksdb_options_set_max_background_compactions(db_opts, 2);
    // rocksdb_options_set_max_write_buffer_number(db_opts, 4);
    // rocksdb_options_set_min_write_buffer_number_to_merge(db_opts, 2);
    // rocksdb_options_set_write_buffer_size(db_opts, 64 * MiB);
    // rocksdb_options_set_num_levels(db_opts, 7);
    // int compression_levels[] = {rocksdb_no_compression, rocksdb_no_compression,
    //                           rocksdb_no_compression, rocksdb_no_compression, 
    //                           rocksdb_no_compression, rocksdb_no_compression, 
    //                           rocksdb_no_compression};
    // rocksdb_options_set_compression_per_level(db_opts, compression_levels, 7);
    // // rocksdb_options_set_row_cache(db_opts, )
    // rocksdb_options_set_enable_pipelined_write(db_opts, false);
    // rocksdb_options_set_target_file_size_base(db_opts, 128 * MiB);
    // rocksdb_options_set_max_manifest_file_size(db_opts, 64 * MiB);
    // rocksdb_options_set_max_log_file_size(db_opts, 256 * MiB);
    // rocksdb_options_set_keep_log_file_num(db_opts, 12);
    // // rocksdb_options_set_WAL_ttl_seconds(db_opts, 3*3600);
    // // rocksdb_options_set_WAL_size_limit_MB(db_opts, 16384);
    // rocksdb_options_set_max_total_wal_size(db_opts, 64*4*2);
    // rocksdb_options_set_dump_malloc_stats(db_opts, true);
    // rocksdb_ratelimiter_t* rate_limiter = rocksdb_ratelimiter_create(1024 *1000 * MiB, 100 * 1000, 10);
    // rocksdb_options_set_ratelimiter(db_opts, rate_limiter);
    // rocksdb_options_set_blob_compaction_readahead_size(db_opts, 2*MiB);
    // rocksdb_options_set_level0_slowdown_writes_trigger(db_opts, 20);
    // rocksdb_options_set_level0_stop_writes_trigger(db_opts, 40);
    // rocksdb_options_set_level0_file_num_compaction_trigger(db_opts, 1);
    // rocksdb_options_set_max_bytes_for_level_base(db_opts, 256 * MiB);
    // rocksdb_options_set_max_bytes_for_level_multiplier(db_opts, 10);
    // rocksdb_options_set_level_compaction_dynamic_level_bytes(db_opts, false);    
    
    // rocksdb_options_set_ttl(db_opts, 10);
    ropts = initReadOpts();
    rocksdb_t* db = openDb(db_opts, "data.rocks/0");


    wopts = initWriteOpts();
    char key[100];
    char* value = malloc(VALUE_SIZE + 1);
    value[VALUE_SIZE] = '\0';
    // memcpy(value, "abc", 3);
    // create_random_value(value, VALUE_SIZE);
    fopts = initFlushOpts();
    long long start_time = ustime();
    long long count = 1000L;
    
    for (long long i = 0; i < count;i++) {
        int key_len = encode(&key, "key", 0);
        sprintf(value, "ab%lld", i);
        writeDb(db, wopts, key, key_len,  value, VALUE_SIZE);
        flushDB(db, fopts);
    }
    
    // rocksdb_compact_range(db, NULL, 0, NULL, 0);
    long long end_time = ustime();
    printf("write once %lldus\n", (end_time - start_time) / count);
    sleep(10);
    free(value);
    rocksdb_options_destroy(db_opts);
    rocksdb_close(db);
    return 1;
}