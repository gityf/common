include ../Makefile.comm

OBJDIR = ./obj/net/
INC += -I../
INC += -I./

SOURCES = $(foreach d,. ./http,$(wildcard $(d)/*.cpp))

OBJS = $(patsubst %.cpp, %.o, $(SOURCES))

CC += $(SHARED_FLAG)
all : libnet.a

libnet.a : $(OBJS)
	ar -rus $@ $^
	#rm -rf $(OBJS)
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using libnet.a             |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

clean:
	rm -rf *.o *.a

install:
	
