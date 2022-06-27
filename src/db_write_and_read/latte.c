//jemalloc
#include <rocksdb/c.h>
#include "../utils/latte_utils.h"
#include "../utils/rocksdb_utils.h"


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

#define VALUE_SIZE (1024 * 1024)
int main() {

    rocksdb_options_t* db_opts;
    rocksdb_cache_t* block_cache;
    rocksdb_writeoptions_t* wopts;
    rocksdb_readoptions_t* ropts;
    
    struct rocksdb_block_based_table_options_t *block_opts ;

    db_opts = initDbOpts();
    ropts = initReadOpts();
    rocksdb_t* db = openDb(db_opts, "data.rocks/0");


    wopts = initWriteOpts();
    char key[100];
    char* value = malloc(VALUE_SIZE + 1);
    value[VALUE_SIZE] = '\0';
    create_random_value(value, VALUE_SIZE);
    long long start_time = ustime();
    long long count = 10000000L;
    for (long long i = 0; i < 10000000L;i++) {
        int key_len = encode(&key, "key", i);
        writeDb(db, wopts, key, key_len,  value, VALUE_SIZE);
    }
    long long end_time = ustime();
    printf("write once %lldus", (end_time - start_time) / count);

    free(value);
    rocksdb_options_destroy(db_opts);
    rocksdb_close(db);
    return 1;
}