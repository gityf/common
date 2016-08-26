#!/bin/sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/ssl/lib
g++ -g -o ssl_server ssl_server.cpp -I/usr/local/ssl/include -L/usr/local/ssl/lib -lssl -lcrypto -ldl

