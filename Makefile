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
	install -v -D -m 644 ./dosu.restrict $(DESTDIR)/etc/dosu.restrict

clean:
	@rm -rf dosu *.o



kz_erch.o: kz_erch.c kz_erch.h
jconf.o: jconf.c nxjson.h kz_erch.h
args-check.o: args-check.c kz_erch.h
path-check.o: path-check.c kz_erch.h
opts-check.o: opts-check.c kz_erch.h
grant-access.o: grant-access.c kz_erch.h jconf.h args-check.h path-check.h opts-check.h
dosu.o: dosu.c kz_erch.h grant-access.h jconf.h
