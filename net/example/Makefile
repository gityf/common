include ../../Makefile.comm
MODULE = ../../libcommon.a
    
LIB = -lstdc++ -lpthread

INC  = -I../..

all : echo   http_echo httpserver_test httpserver_test2

echo : echo.o $(MODULE)
	$(CC) -o $@ $< $(LIB) $(MODULE)
httpserver_test : httpserver_test.o $(MODULE)
	$(CC) -o $@ $< $(LIB) $(MODULE)
httpserver_test2 : httpserver_test2.o $(MODULE)
	$(CC) -o $@ $< $(LIB) $(MODULE)

http_echo : http_echo.o $(MODULE)
	$(CC) -o $@ $< $(LIB) $(MODULE)
	@echo ""
	@echo "+--------------------------------------------+"
	@echo "|            Finish compilation              |"
	@echo "+--------------------------------------------+"
	@echo "|         Thanks using example               |"
	@echo "|   copyright(c)Wang Yaofu  voipman@qq.com   |"
	@echo "+--------------------------------------------+"

clean:
	rm -rf *.o *.a

install :
