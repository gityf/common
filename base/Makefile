include ../Makefile.comm

INC += -I../
INC += -I./
OBJDIR = ./obj/base/

SOURCES = $(foreach d,.,$(wildcard $(d)/*.cpp))
OBJS = $(patsubst %.cpp, %.o, $(SOURCES))

CC += $(SHARED_FLAG)
all : libbase.a

libbase.a : $(OBJS)
	ar -rus $@ $^
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using libbase.a             |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

clean:
	rm -rf *.o *.a

install:
	
