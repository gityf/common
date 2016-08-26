include ../../Makefile.comm

INC += -I../
INC += -I./

SOURCES = $(foreach d,.,$(wildcard $(d)/*.cpp))
OBJS = $(patsubst %.cpp, %.o, $(SOURCES))

CC += $(SHARED_FLAG)
all : libgziputil.a

libgziputil.a : $(OBJS)
	ar -rus $@ $^
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using libgziputil.a         |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

clean:
	rm -rf *.o *.a

install:
	
