include ../Makefile.comm

OBJDIR = ./obj/posix/
INC += -I../
INC += -I./

SOURCES = $(foreach d,.,$(wildcard $(d)/*.cpp))
OBJS = $(patsubst %.cpp, %.o, $(SOURCES))

CC += $(SHARED_FLAG)
all : libposix.a

libposix.a : $(OBJS)
	ar -rus $@ $^
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using libposix.a            |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

clean:
	rm -rf *.o *.a

install:
	
