OPTIMIZATION?=-O0
OPT=$(OPTIMIZATION)
# Detect if the compiler supports C11 _Atomic
C11_ATOMIC := $(shell sh -c 'echo "\#include <stdatomic.h>" > foo.c; \
	$(CC) -std=c11 -c foo.c -o foo.o > /dev/null 2>&1; \
	if [ -f foo.o ]; then echo "yes"; rm foo.o; fi; rm foo.c')
ifeq ($(C11_ATOMIC),yes)
	STD+=-std=c11
else
	STD+=-std=c99
endif
DEBUG=-g -ggdb -fno-omit-frame-pointer
WARN=-Wall -W -Wno-missing-field-initializers
FINAL_CFLAGS=$(STD) $(WARN) $(OPT)
FINAL_CFLAGS+=-I../../deps/rocksdb/include
LATTE_CC=$(CC) $(FINAL_CFLAGS)
FINAL_LDFLAGS=$(LDFLAGS) $(LATTE_LDFLAGS) $(DEBUG) $(OPT)
LATTE_LD=$(LINK)$(CC) $(FINAL_LDFLAGS)
FINAL_LIBS=../../deps/rocksdb/librocksdb.a -lm -lrt -lstdc++ -pthread -lsnappy -lz 

%.o: %.c
	$(LATTE_CC) -O0 -MMD -o $@ -c $<


##### diff
latte: latte.o 
	$(LATTE_LD)  -o $@ $< $(FINAL_LIBS) 

####
leaks:latte
	export MallocStackLogging=1
	leaks --atExit -- ./latte

leaks-exclude:latte
	export MallocStackLogging=1
	export MallocScribble=1  
	export MallocStackLoggingNoCompact=1
	leaks --atExit -- ./latte  > memleak.log
	export MallocScribble=0  
	export MallocStackLogging=0 
	export MallocStackLoggingNoCompact=0

build:latte 
	@echo "build ok"

clean:
	@rm -rf *.o *.d
	@rm -rf latte
	@rm -rf data.rocks/*