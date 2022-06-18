
#include <stdlib.h>
#include <stdio.h>
#include <rocksdb/c.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define ROCKS_DATA "data.rocks"
#define WNOHANG 0x00000001
static void Free(char** ptr) {
  if (*ptr) {
    free(*ptr);
    *ptr = NULL;
  }
}

rocksdb_options_t* initDbOpts() {
    rocksdb_options_t* options = rocksdb_options_create();
    rocksdb_options_set_create_if_missing(options, 1); 
    rocksdb_options_enable_statistics(options);
    rocksdb_options_set_stats_dump_period_sec(options, 60);
    rocksdb_options_set_max_write_buffer_number(options, 6);
    rocksdb_options_set_max_bytes_for_level_base(options, 512*1024*1024);
    //
    // rocksdb_options_set_max_open_files(db_opts, 10);
    // rocksdb_options_set_paranoid_checks(options, 1);

    // rocksdb_options_set_comparator(options, cmp);
    rocksdb_options_set_error_if_exists(options, 1);
    // rocksdb_options_set_env(options, env);
    rocksdb_options_set_info_log(options, NULL);
    rocksdb_options_set_write_buffer_size(options, 100000);
    rocksdb_options_set_paranoid_checks(options, 1);
    rocksdb_options_set_max_open_files(options, 10);
    rocksdb_options_set_base_background_compactions(options, 1);
    rocksdb_options_set_compression(options, rocksdb_no_compression);
    rocksdb_options_set_compression_options(options, -14, -1, 0, 0);
    
    return options;
}

void* write_and_read(void* p) {
    printf("%p \n", p);
    char* err;
    rocksdb_readoptions_t* ropts;
    rocksdb_writeoptions_t* wopts;
    rocksdb_t *db = (rocksdb_t*)p;
    
    ropts = rocksdb_readoptions_create();
    rocksdb_readoptions_set_verify_checksums(ropts, 0);
    rocksdb_readoptions_set_fill_cache(ropts, 0);

    wopts = rocksdb_writeoptions_create();
    rocksdb_writeoptions_disable_WAL(wopts, 1);
    //write data
    rocksdb_put(db, wopts, "key", 3, "value", 5, err);
    if (err != NULL) {
        printf("rocksdb_put fail: %s\n", strerror(err));
        return err;
    }
    int val_len = 0;
    char* val;
    val = rocksdb_get(db, ropts, "key", 3, &val_len, err);
    printf("get key val:%s\n", val);
    Free(&val);
    return NULL;
}

void* write_and_read2(void* p) {
    printf("%p \n", p);
    char* err;
    rocksdb_readoptions_t* ropts;
    rocksdb_writeoptions_t* wopts;
    rocksdb_t *db = (rocksdb_t*)p;
    
    ropts = rocksdb_readoptions_create();
    rocksdb_readoptions_set_verify_checksums(ropts, 0);
    rocksdb_readoptions_set_fill_cache(ropts, 0);

    wopts = rocksdb_writeoptions_create();
    rocksdb_writeoptions_disable_WAL(wopts, 1);
    //write data
    rocksdb_put(db, wopts, "key", 3, "value2", 6, err);
    if (err != NULL) {
        printf("rocksdb_put fail: %s\n", strerror(err));
        return err;
    }
    int val_len = 0;
    char* val;
    val = rocksdb_get(db, ropts, "key", 3, &val_len, err);
    printf("get key val:%s\n", val);
    Free(&val);
    return NULL;
}

void child_process_read_checkpoint(rocksdb_options_t* options, char* dbcheckpointname) {
    sleep(5);
    char* err = NULL;
    printf("hello\n");
    rocksdb_readoptions_t* ropts;
    
    rocksdb_options_set_error_if_exists(options, 0);
    printf("hello2\n");
    rocksdb_t* db = rocksdb_open(options, dbcheckpointname, &err);
    printf("hello3\n");
    if (err != NULL) {
        printf("rocksdb_open dir2 fail :%s \n", err);
        return;
    } 
    printf("hello4\n");
    ropts = rocksdb_readoptions_create();
    rocksdb_readoptions_set_verify_checksums(ropts, 0);
    rocksdb_readoptions_set_fill_cache(ropts, 0);
    int val_len = 0;
    char* val = NULL;
    printf("hello5\n");
    val = rocksdb_get(db, ropts, "key", 3, &val_len, err);
    if (err != NULL) {
        printf("rocksdb_get key fail :%s \n", err);
    } else {
        printf("get val:%s\n", val);
        Free(&val);
    }
    printf("hello6\n");
    rocksdb_close(db);
    // char* val2 = NULL;
    // err = NULL;
    // val2 = rocksdb_get(db, ropts, "key1", 4, &val_len, err);
    // printf("hello7\n");
    // if (err != NULL) {
    //     printf("rocksdb_get key1 fail :%s \n", err);
    // } else {
    //     printf("get val:%s\n", val);
    //     Free(&val);
    // }
    
    // printf("hello8\n");
}
int main() {
    rocksdb_options_t* db_opts;
    rocksdb_cache_t* block_cache;
    rocksdb_readoptions_t* ropts;
    rocksdb_writeoptions_t* wopts;
    struct rocksdb_block_based_table_options_t *block_opts ;

    db_opts = initDbOpts();

    block_opts = rocksdb_block_based_options_create();
    rocksdb_block_based_options_set_block_size(block_opts, 8192);
    block_cache = rocksdb_cache_create_lru(1*1024*1024);
    rocksdb_block_based_options_set_block_cache(block_opts, block_cache);
    rocksdb_block_based_options_set_cache_index_and_filter_blocks(block_opts, 0);
    rocksdb_options_set_block_based_table_factory(db_opts, block_opts);

    rocksdb_options_optimize_for_point_lookup(db_opts, 1);
    rocksdb_options_optimize_level_style_compaction(db_opts, 256*1024*1024);
    rocksdb_options_set_max_background_compactions(db_opts, 4); /* default 1 */
    rocksdb_options_compaction_readahead_size(db_opts, 2*1024*1024); /* default 0 */
    rocksdb_options_set_optimize_filters_for_hits(db_opts, 1); /* default false */

    ropts = rocksdb_readoptions_create();
    rocksdb_readoptions_set_verify_checksums(ropts, 0);
    rocksdb_readoptions_set_fill_cache(ropts, 0);

    wopts = rocksdb_writeoptions_create();
    rocksdb_writeoptions_disable_WAL(wopts, 1);


    struct stat statbuf;
    if (!stat(ROCKS_DATA, &statbuf) && S_ISDIR(statbuf.st_mode)) {
        /* "data.rocks" folder already exists, no need to create */
    } else if (mkdir(ROCKS_DATA, 0755)) {
        printf("mkdir %s fail: %s !\n", ROCKS_DATA, strerror(errno));
        return -1;
    }
    char* err = NULL;
    char dir[512];
    snprintf(dir, 512, "%s/%d", ROCKS_DATA, 0);
    printf("dir: %s\n", dir);
    rocksdb_t* db = rocksdb_open(db_opts, dir, &err);
    if (err != NULL) {
        printf("rocksdb_open fail: %s\n", strerror(err));
        return -1;
    }
    pthread_t tid;
    if (pthread_create(&tid, NULL, write_and_read, db) != 0) {
        printf("create pthread write fail\n");
        return -1;
    }
    // int result = 0;
    if (pthread_join(tid, &err)) {
        printf("create pthread join fail\n");
        return -1;
    }

    if (err != NULL) {
        printf("write fail: %s\n", strerror(err));
        return -1;
    }
    printf("pthread_create\n");
    // printf("join result: %d\n", result);

    char dir2[512];
    snprintf(dir2, 512, "%s/%d", "/Users/zhouguodong/Documents/myself/latte_rocksdb_demo/src/checkpoint/data.rocks", 1);
    printf("dir: %s\n", dir2);
    // rocksdb_destroy_db(db_opts, dir2, &err);
    
    rocksdb_checkpoint_t* checkpoint = rocksdb_checkpoint_object_create(db, &err);
    if (err != NULL) {
        printf("rocksdb_checkpoint_object_create fail: %s\n", strerror(err));
        return -1;
    }
    
    
    rocksdb_checkpoint_create(checkpoint, dir2, 0, &err);
    if (err != NULL) {
        printf("rocksdb_checkpoint_create fail: %s\n", strerror(err));
        return -1;
    }

    int childpid = 0;
    if ((childpid = fork()) == 0) {
        /* Child */
        child_process_read_checkpoint(db_opts, dir2);
    } else {
        /* Parent */
        // write_and_read2(db);
        if (pthread_create(&tid, NULL, write_and_read2, db) != 0) {
            printf("create pthread write fail\n");
            return -1;
        }
        // int result = 0;
        if (pthread_join(tid, &err)) {
            printf("create pthread join fail\n");
            return -1;
        }
        int pid;
        int statloc = 0;
        while (1) {
            if ((pid = waitpid(-1, &statloc, WNOHANG)) != 0) {
            if ( pid == childpid) {
                    //Child over
                    printf("fork over\n");
                    // rocksdb_checkpoint_object_destroy(checkpoint);
                    // rocksdb_destroy_db(db_opts, dir2, &err);
                    // if(err != NULL) {
                    //     printf("rocksdb_destroy_db fail: %s\n", strerror(err));
                    //     return -1;
                    // }
                    break;
                }
                sleep(1);
            }
        }
        
    }

    return 0;
}