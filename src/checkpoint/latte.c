
#include <stdlib.h>
#include <stdio.h>
#include <rocksdb/c.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "../utils/rocksdb_utils.h"
#include "../utils/latte_utils.h"
#define VALUE_SIZE ( 1024 * 1024)
#define WNOHANG 0x00000001


rocksdb_t* openDb_for_readonly(rocksdb_options_t* options, char* dir) {
    char* err = NULL;
    // rocksdb_t* db = rocksdb_open_with_ttl(options, dir, 1, &err);
    rocksdb_t* db = rocksdb_open_for_read_only(options, dir, 0, &err);
    printf("rocksdb open %s\n", dir);
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




void create_random_value(char* value, int size) {
    for(int i = 0 ; i < size; i++) {
        value[i] = rand() % 10000;
    }
}




void* write_data(void* p) {
    rocksdb_t * db = (rocksdb_t*)p;
    rocksdb_writeoptions_t* wopts;
    rocksdb_readoptions_t* ropts;
    rocksdb_flushoptions_t* fopts;
    wopts = initWriteOpts();
    ropts = initReadOpts();

    if(writeDb(db, wopts, "key", 3, "value", 5) == 0) {
        return "write key fail";
    };
    fopts = initFlushOpts();
    // flushDB(db, fopts);
    int len = 0;
    char* value = readDb(db, ropts, "key",3, &len);
    if (value == NULL) {
        return "read key fail";
    }
    if(strncmp(value, "value", len) == 0) {
        printf("set key value ok!\n");
    } else {
        printf("set key fail value:%s value_size:%d\n", value, len);
    }
    free(value);
    return NULL;
}




void* write_data2(void* p) {
    rocksdb_t * db = (rocksdb_t*)p;
    rocksdb_writeoptions_t* wopts;
    rocksdb_readoptions_t* ropts;
    rocksdb_flushoptions_t* fopts;
    wopts = initWriteOpts();
    ropts = initReadOpts();

    if(writeDb(db, wopts, "key", 3, "value1", 6) == 0) {
        return "write key fail";
    };
    fopts = initFlushOpts();
    flushDB(db, fopts);
    int len = 0;
    char* value = readDb(db, ropts, "key",3, &len);
    if (value == NULL) {
        return "read key fail";
    }
    
    if(strncmp(value, "value1", len) == 0) {
        printf("set key value1 ok!\n");
    } else {
        printf("set key fail value:%s  len: %d\n", value, len);
    }
    free(value);
    return NULL;
    
}

typedef void *(*__start_routine) (void *);

int execThreadTask(__start_routine func, rocksdb_t* db, pthread_t* pid) {

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








int read_checkpoint(char* checkpoint_dir) {
    rocksdb_options_t* db_opts;
    rocksdb_readoptions_t* ropts;
    db_opts = initDbOpts();
    rocksdb_t* db = openDb_for_readonly(db_opts, checkpoint_dir);
    if (db == NULL) {
        printf("open checkpoint db fail\n");
        return 0;
    }
    ropts = initReadOpts();
    int len = 0;
    char* value = readDb(db, ropts, "key", 3, &len);
    if (value == NULL) {
        printf("read checkpoint key fail\n");
        return 0;
    }
    printf("read checkpoint value: %s, value_size: %d\n", value, len);
    free(value);
    return 1;
} 
int main() {
    rocksdb_options_t* db_opts;
    rocksdb_readoptions_t* ropts;
    
    db_opts = initDbOpts();
    ropts = initReadOpts();
    

    rocksdb_t* db = openDb(db_opts, "data.rocks/0");
    long long start_time, end_time;
    start_time = ustime();

    if (execThreadTask(write_data, db, NULL) != 1) {
        printf(" write data fail2\n");
        return -1;
    }
    
    rocksdb_checkpoint_t* checkpoint =  createCheckpoint(db, "data.rocks/1", 0);
    end_time = ustime();
    printf("checkpoint use time :%lld\n", end_time - start_time);

    int childpid = 0;
    if((childpid = fork()) == 0) {
        /* child */
        sleep(10);
        read_checkpoint("data.rocks/1");
    } else {
        if (execThreadTask(write_data2, db, NULL) != 1) {
            printf(" write data fail2\n");
            return -1;
        }
        int pid;
        int statloc = 0;
        while (1) {
            if ((pid = waitpid(-1, &statloc, WNOHANG)) != 0) {
            if ( pid == childpid) {
                    //Child over
                    printf("fork over\n");
                    rocksdb_checkpoint_object_destroy(checkpoint);
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