include ../Makefile.comm

INC = -I./

OBJ = sslsocket.o

libsockssl.a : $(OBJ)
	ar -rus $@ $^

clean:
	rm -rf *.o *.a

