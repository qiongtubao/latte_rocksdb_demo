#include <rocksdb/c.h>

//配置
rocksdb_options_t* initDbOpts();
rocksdb_readoptions_t* initReadOpts();
rocksdb_writeoptions_t* initWriteOpts();
rocksdb_flushoptions_t* initFlushOpts();

//常用操作
rocksdb_t* openDb(rocksdb_options_t* options, char* dir);
char* readDb(rocksdb_t* db, rocksdb_readoptions_t* ropts, char* key, int key_size, int* value_size) ;
int flushDB(rocksdb_t* db, rocksdb_flushoptions_t* fopts);
int writeDb(rocksdb_t* db, rocksdb_writeoptions_t* wopts, char* key, int key_size, char* value, int value_size);



//不常用操作
rocksdb_checkpoint_t* createCheckpoint(rocksdb_t* db, char* dir, uint64_t log_size_for_flush);
