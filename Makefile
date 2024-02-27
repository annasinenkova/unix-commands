CC = gcc
CFLAGS = -O2 -ftrapv -fsanitize=address,undefined -Wall -Werror -Wformat-security -Wignored-qualifiers -Winit-self -Wswitch-default -Wfloat-equal -Wshadow -Wpointer-arith -Wtype-limits -Wempty-body -Wlogical-op -Wstrict-prototypes -Wold-style-declaration -Wold-style-definition -Wmissing-parameter-type -Wmissing-field-initializers -Wnested-externs -Wno-pointer-sign -std=gnu11 -lm
LCFLAGS = -fsanitize=address,undefined

all: cat wc cp clean

cat: cat.o lib.o
	$(CC) $(LCFLAGS) cat.o lib.o -o cat
	
cat.o: cat.c lib.h
	$(CC) $(CFLAGS) -c cat.c -o cat.o
	
wc: wc.o lib.o
	$(CC) $(LCFLAGS) wc.o lib.o -o wc
	
wc.o: wc.c lib.h
	$(CC) $(CFLAGS) -c wc.c -o wc.o
	
cp: cp.o lib.o
	$(CC) $(LCFLAGS) cp.o lib.o -o cp

cp.o: cp.c lib.h
	$(CC) $(CFLAGS) -c cp.c -o cp.o
	
lib.o: lib.c lib.h
	$(CC) $(CFLAGS) -c lib.c -o lib.o
	
clean:
	rm lib.o cp.o wc.o cat.o
