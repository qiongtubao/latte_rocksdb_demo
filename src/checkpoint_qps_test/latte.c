
#include <stdlib.h>
#include <stdio.h>
#include <rocksdb/c.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#define VALUE_SIZE ( 1024 * 1024)

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

static long long ustime(void) {
    struct timeval tv;
    long long ust;

    gettimeofday(&tv, NULL);
    ust = ((long long)tv.tv_sec)*1000000;
    ust += tv.tv_usec;
    return ust;
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


rocksdb_t* openDb(rocksdb_options_t* options, char* dir) {
    char* err = NULL;
    rocksdb_t* db = rocksdb_open(options, dir, &err);
    if (err != NULL) {
        printf("open db %s fail\n", dir);
        return NULL;
    }
    return db;
}

int encode(char* buf, char* pre, long long index) {
    int len = sprintf(buf, "%s-%lld", pre, index);
    buf[len] = '\0';
    return len;
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


void create_random_value(char* value, int size) {
    for(int i = 0 ; i < size; i++) {
        value[i] = rand() % 10000;
    }
}

void* write_data(void* p) {
    rocksdb_t* db = (rocksdb_t*)p;
    rocksdb_writeoptions_t* wopts;
    wopts = initWriteOpts();

    long long dbsize = (1024L * 1024 * 1024 * 10)/ VALUE_SIZE;
    printf("write data size: %lld\n", dbsize);
    char key[1000];
    char* value = malloc(VALUE_SIZE + 1);
    value[VALUE_SIZE] = '\0';
    create_random_value(value, VALUE_SIZE);
    for(long long i = 0; i < dbsize; i++) {
        int key_size = encode(&key, "data", i);    
        if (0 == writeDb(db, wopts, key, key_size, value, VALUE_SIZE)) {
            free(value);
            return "write data fail";
        }
    }
    free(value);
    return NULL;
    
}




void* write_data2(void* p) {
    rocksdb_t* db = (rocksdb_t*)p;
    rocksdb_writeoptions_t* wopts;
    wopts = initWriteOpts();

    
    char key[1000];

    
    long long index = 0;
    long long qps = 1000;
    //1s 1000qps  => 100ms
    long long once_use_time = 100;
 
    char* value = malloc(VALUE_SIZE + 1);
    value[VALUE_SIZE] = '\0';
    long long dbsize = (1024L * 1024L * 1024L * 10L)/ VALUE_SIZE;
    while (1) {
        long long start_time = ustime();
        int key_size = encode(&key, "data",  rand() % dbsize);
        create_random_value(value, VALUE_SIZE);
        if (0 == writeDb(db, wopts, key, key_size, value, VALUE_SIZE)) {
            free(value);
            return "write db2 fail";
        }
        // printf("rewrite key %s, value %d%d\n", key, value[0], value[1]);
        index++;
        long long end_time = ustime();
        long long usetime = end_time - start_time;
        if (usetime < once_use_time) {
            // printf("usleep %lld\n", once_use_time - usetime);
            usleep(once_use_time - usetime);
        }
    }
    free(value);
    return NULL;
    
}

typedef void *(*__start_routine) (void *);

int execThreadTask(__start_routine func, rocksdb_t* db, int* pid) {

    char* err;
    pthread_t tid;
    if (pthread_create(&tid, NULL, func, db) != 0) {
        printf("create pthread write fail\n");
        return -1;
    }
    
    if (pid == NULL) {
        if (pthread_join(tid, (void**)&err)) {
            printf("create pthread join fail\n");
            return -1;
        }
    } else {
        *pid = tid;
    }
    return 1;
}




rocksdb_checkpoint_t* create_checkpoint(rocksdb_t* db, char* dir, uint64_t log_size_for_flush) {
    char* err = NULL;
    rocksdb_checkpoint_t* checkpoint = rocksdb_checkpoint_object_create(db, &err);
    if (err != NULL) {
        printf("checkpoint create fail :%s\n", err);
        return NULL;
    }
    rocksdb_checkpoint_create(checkpoint, dir, log_size_for_flush, &err);
    return checkpoint;
}

int main() {
    rocksdb_options_t* db_opts;
    rocksdb_cache_t* block_cache;
    rocksdb_readoptions_t* ropts;
    
    struct rocksdb_block_based_table_options_t *block_opts ;

    db_opts = initDbOpts();
    ropts = initReadOpts();
    

    rocksdb_t* db = openDb(db_opts, "data.rocks/0");
    long long start_time, end_time;
    start_time = ustime();

    if (execThreadTask(write_data, db, NULL) != 1) {
        printf(" write data fail2\n");
        return -1;
    }
    end_time = ustime();
    printf("use time :%lld\n", end_time - start_time);
    int pid = 0;
    if (execThreadTask(write_data2, db, &pid) != 1) {
        printf("write2 data fail\n");
        return -1;
    }

    start_time = ustime();
    rocksdb_checkpoint_t* checkpoint =  create_checkpoint(db, "data.rocks/1", 0);
    end_time = ustime();
    printf("checkpoint use time :%lld\n", end_time - start_time);
    start_time = ustime();
    rocksdb_t* db1 = openDb(db_opts, "data.rocks/1");
    end_time = ustime();
    return 0;
}