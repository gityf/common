include ../../Makefile.comm
PROJECT=dlloader
INC += -I../..
SRCS = . ..
SOURCES = $(foreach d,$(SRCS),$(wildcard $(d)/*.cpp))
OBJS = $(patsubst %.cpp, %.o, $(SOURCES))

CC += $(SHARED_FLAG)
all : dlloader

$(PROJECT): $(OBJS)
	$(CCC) -o $@ $(OBJS) -ldl -lstdc++
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using dlloader              |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

clean:
	rm -rf *.o $(PROJECT)

