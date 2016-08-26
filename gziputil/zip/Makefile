include ../../../Makefile.comm

INC += -I../
INC += -I./

SOURCES = $(foreach d,.,$(wildcard $(d)/*.c))
OBJS = $(patsubst %.c, %.o, $(SOURCES))

CC += $(SHARED_FLAG)
all : libzip.a

libzip.a : $(OBJS)
	ar -rus $@ $^
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using libzip.a              |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

clean:
	rm -rf *.o *.a

install:
	
