CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres
OBJDIR=objs


libcal_src = $(wildcard libcal/*.c)
libcal_objects = $(patsubst %.c, $(OBJDIR)/%.o, $(libcal_src))
basefunc_src = $(wildcard basefunc/*.c)
basefunc_objects = $(patsubst %.c, $(OBJDIR)/%.o, $(basefunc_src))

all : libcal basefunc ecc
	
libcal : init_libcal $(libcal_objects)
	@$(AR)  -r -s lib/libcal.a $(libcal_objects) >/dev/null 2>&1

init_libcal :
	@echo "build libcal"
	@mkdir -p $(OBJDIR)/libcal
	@mkdir -p lib

$(OBJDIR)/libcal/%.o : libcal/%.c
	@$(CC) -c $< -o $@

basefunc : init_basefunc $(basefunc_objects)
	@$(AR)  -r -s lib/basefunc.a $(basefunc_objects) >/dev/null 2>&1

init_basefunc :
	@echo "build basefunc"
	@mkdir -p $(OBJDIR)/basefunc
	@mkdir -p lib

$(OBJDIR)/basefunc/%.o : basefunc/%.c
	@$(CC) -c $< -o $@

ecc : libcal basefunc
	@echo "build ecc"
	@mkdir -p bin
	@$(CXX) -std=c++11 -c cli/cli.cpp -o objs/cli.o
	@$(CXX) -std=c++11 -o bin/ecc objs/cli.o -lgmp -pthread -lssl -lcrypto lib/basefunc.a lib/libcal.a
	@echo "build success"

clean : clean_libcal clean_basefunc clean_ecc
	
clean_libcal :
	@rm -f lib/libcal.a $(libcal_objects)

clean_basefunc :
	@rm -f lib/basefunc.a $(basefunc_objects)

clean_ecc :
	@rm -f bin/ecc objs/cli.o

.PHONY : clean clean_libcal clean_basefunc
