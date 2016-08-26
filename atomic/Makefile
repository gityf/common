PROJECT = bin/atomic_test

#CC=gcc -std=c++11 -g -O3 $(CFLAGS)
CC=gcc -g

SHARED_FLAG = -fPIC -shared

OBJDIR = ../obj/atomic/
SRCS  = test
SRCS += .
SRCS += ../base

LIB  = -lstdc++ -lpthread

INC  = -I.
INC += -I..

CPP_SRCS = $(foreach d,$(SRCS),$(wildcard $(d)/*.cpp))
OBJS = $(addprefix $(OBJDIR), $(patsubst %.cpp, %.o, $(CPP_SRCS)))

all : obj $(PROJECT)
obj :
	mkdir -p $(addprefix $(OBJDIR), $(SRCS))

$(PROJECT) : $(OBJS) $(MODULE)
	test -d ./bin || mkdir -p ./bin
	$(CC) -o $@ $(OBJS) $(LIB)
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|      Finish compilation atomic             |"
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
