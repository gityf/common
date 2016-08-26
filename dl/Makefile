include ../Makefile.comm

OBJDIR = ./obj/dl/
INC += -I../

SOURCES = $(foreach d,.,$(wildcard $(d)/*.cpp))
OBJS = $(patsubst %.cpp, %.o, $(SOURCES))

CC += $(SHARED_FLAG)
all : libdlload.a 
libdlload.a : $(OBJS)
	ar -rus $@ $^
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using libdlload.a           |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

clean:
	rm -rf *.o *.a

