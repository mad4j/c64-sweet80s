
OUT_NAME=sweet80s

KOA_LIST := ${patsubst %.koa,%,$(wildcard *.koa)}
GZP_LIST := ${patsubst %.koa.gz,%,$(wildcard *.koa.gz)}

## HOW TO crate a gzipped Koala file
## 1. tail -c +3 ylenia3.koa | gzip > temp.tmp
## 2. printf "\x01\02" | cat - temp.tmp > ylenia3.koa.gz
## 3. rm temp.tmp

.PHONY: all d64 prg clean

all: prg d64

prg:
	cl65 -t c64 -O $(OUT_NAME).c -o $(OUT_NAME).prg
	rm -f *.o

d64:
	c1541 -format $(OUT_NAME),AA d64 $(OUT_NAME).d64
	c1541 -attach $(OUT_NAME).d64 -write $(OUT_NAME).prg $(OUT_NAME)
	@for f in $(GZP_LIST) ; do \
		c1541 -attach $(OUT_NAME).d64 -write $$f.koa.gz %$$f ; \
	done
	@for f in $(KOA_LIST) ; do \
		c1541 -attach $(OUT_NAME).d64 -write $$f.koa !$$f ; \
	done
	c1541 -attach $(OUT_NAME).d64 -dir

clean:
	rm -f *.o *.prg *.sym *.d64
