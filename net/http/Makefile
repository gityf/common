include ../../Makefile.comm

OBJDIR = ./obj/net/http/
INC += -I../../
INC += -I./

SOURCES = $(foreach d,.,$(wildcard $(d)/*.cpp))
OBJS = $(patsubst %.cpp, %.o, $(SOURCES))

CC += $(SHARED_FLAG)
all : libhttp.a

libhttp.a : $(OBJS)
	ar -rus $@ $^
	#rm -rf $(OBJS)
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using libhttp.a             |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

clean:
	rm -rf *.o *.a

install:
	
