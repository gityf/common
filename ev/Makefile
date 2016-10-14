include ../Makefile.comm

OBJDIR = ./obj/ev/
INC = -I../
CC += $(SHARED_FLAG)
OBJS =  eventloop.o
OBJS += eventpoll.o
OBJS += eventhandler.o
OBJS += eventfd.o
OBJS += event_timer.o
OBJS += ngx_rbtree.o

libsockpoll.a : $(OBJS)
	ar -rus $@ $^
	rm -rf $(OBJS)
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using libsockpoll.a         |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"
clean:
	rm -rf *.o *.a

