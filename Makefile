.EXPORT_ALL_VARIABLES:

CC = gcc

CFLAGS = -Wall -Wextra -std=c99 -I. -g -O0
LDFLAGS = -Wall -Wextra

.PHONY: clean

OBJS = $(subst .c,.o,$(wildcard *.c))

CFLAGS += -fpic
LDFLAGS += -fpic -rdynamic

%.o: %.c
	$(CC) $(CFLAGS) -c $<

all: dosu

dosu: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

install: dosu
	install -v -D -m 4710 ./dosu $(DESTDIR)/usr/sbin/dosu
	install -v -D -m 644 ./dosu.conf $(DESTDIR)/etc/dosu.conf

clean:
	@rm -rf dosu *.o



kz_erch.o: kz_erch.c kz_erch.h
dosu.o: dosu.c kz_erch.h
