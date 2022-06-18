OPTIMIZATION?=-O0
OPT=$(OPTIMIZATION)
STD=-std=c99
DEBUG=-g -ggdb -fno-omit-frame-pointer
WARN=-Wall -W -Wno-missing-field-initializers
FINAL_CFLAGS=$(STD) $(WARN) $(OPT)
FINAL_CFLAGS+=-I../../deps/rocksdb/include
LATTE_CC=$(CC) $(FINAL_CFLAGS)
FINAL_LDFLAGS=$(LDFLAGS) $(LATTE_LDFLAGS) $(DEBUG) $(OPT)
LATTE_LD=$(LINK)$(CC) $(FINAL_LDFLAGS)
FINAL_LIBS=-lm  -lstdc++ -pthread  -lz ../../deps/rocksdb/librocksdb.a

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
	@rm -rf latte.o latte
	@rm -rf data.rocks