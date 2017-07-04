ifeq	($(release),1)

FLG := -O3 -fPIC

else

FLG := -O3 -g -fPIC

endif

PRG := bin/libutils.so

SRC := $(wildcard src/*.cpp)

OBJ := $(patsubst %.cpp,%.o,$(SRC))

CC := g++

INC := -I./inc \
	   -I../../../lib/linux/inc/ \
	   -I../../../lib/cross/boost/include/

LIB := -L../../../lib/linux/lib/ -lcconv 

.PHONY: clean

$(PRG): $(OBJ)
	$(CC) -shared -o $@ $^ $(LIB)

%.o: %.cpp
	$(CC) $(FLG) -o $@ -c $< $(INC)

clean:
	rm -f $(OBJ) $(PRG)
