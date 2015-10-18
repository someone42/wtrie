IDIR=.
CC=gcc
CFLAGS=-g -O0 -Wall -I$(IDIR)

ODIR=obj

LIBS=-lstdc++

_DEPS = mtrand.h tinyformat.h uint256.h utilstrencodings.h wtrie.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = mtrand.o uint256.o utilstrencodings.o wtrie.o wtrie_test.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

wtrie_test: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core wtrie_test wtrie_test.exe $(INCDIR)/*~ 
