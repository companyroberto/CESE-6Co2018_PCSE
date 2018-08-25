OS=LINUX
SOC=BCM2835
DRIVER=RPi
CPUFLAGS=-march=armv6zk -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard
CFLAGS=-march=armv6zk -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -Ofast -Wall -pthread 
PREFIX=/usr/local
REMOTE_PREFIX=/usr/local
LIB=rf24
LIBNAME=librf24.so.1.1.6
LIB_VERSION=1.1.6
LIBSYMLINKS=librf24.so.1.1 librf24.so.1 librf24.so
LIBDEPRECATE=librf24-bcm.so
CC=arm-linux-gnueabihf-gcc
CXX=arm-linux-gnueabihf-g++
LIB_DIR=/usr/local/lib
REMOTE_LIB_DIR=/usr/local/lib
HEADER_DIR=/usr/local/include/RF24
REMOTE_HEADER_DIR=/usr/local/include/RF24
DRIVER_DIR=utility/RPi
ARCH_DIR=utility
SHARED_LINKER_FLAGS= -pthread -shared -Wl,-soname,librf24.so.1
LDCONFIG=ldconfig
REMOTE_LDCONFIG=/sbin/ldconfig
EXAMPLES_DIR=/usr/local/bin
REMOTE_EXAMPLES_DIR=/usr/local/bin
BINARY_PREFIX = rf24
LIBS=-l$(LIB)
CPPFLAGS = -g -I.


all: utiles.c serviciosTCP.c sensores.c mensajeria.c conexionTP.c comunicacion.c bufferCircular.c siscocaf_v1

siscocaf_v1 : siscocaf.c
	$(CXX) $(CFLAGS) -I$(HEADER_DIR)/.. -I.. -L$(LIB_DIR) $(CPPFLAGS) utiles.c serviciosTCP.c sensores.c mensajeria.c conexionTP.c comunicacion.c bufferCircular.c siscocaf.c $(LIBS) -lpthread -o siscocaf_v1

clean:
	rm -rf siscocaf_v1
