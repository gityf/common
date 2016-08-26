PROJECT = libcommon.a

#CC=gcc -std=c++11 -g -O3 $(CFLAGS)
CC=gcc -g -std=c++11
SHARED_FLAG = -fPIC -shared

OBJDIR = ./obj/
SRCS  = base
SRCS += ev
SRCS += ipc
SRCS += net
SRCS += net/http
SRCS += posix
SRCS += scs

LIB += -lstdc++ -lpthread

INC  = -I.

CPP_SRCS = $(foreach d,$(SRCS),$(wildcard $(d)/*.cpp))
OBJS = $(addprefix $(OBJDIR), $(patsubst %.cpp, %.o, $(CPP_SRCS)))

all : obj $(PROJECT)
obj :
	mkdir -p $(addprefix $(OBJDIR), $(SRCS))

$(PROJECT) : $(OBJS)
	test -d ./bin || mkdir -p ./bin
	ar -rus $@ $^
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|     Finish compilation libcommon           |"
	@echo "+--------------------------------------------+"
	@echo "|      copyright(c) voipman@qq.com           |"
	@echo "+--------------------------------------------+"

clean:
	rm -rf $(OBJS)


$(OBJDIR)%.o : %.cpp
	$(CC) $(INC) -c $< -o $@
$(OBJDIR)%.o : %.c
	$(CC) $(INC) -c $< -o $@
$(OBJDIR)%.o : %.cc
	$(CC) $(INC) -c $< -o $@
