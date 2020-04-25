
OUT_NAME=sweet80s

KOA_LIST := ${patsubst %.koa,%,$(wildcard *.koa)}
ZZ_LIST := ${patsubst %.koa.zz,%,$(wildcard *.koa.zz)}

## HOW TO crate a gzipped Koala file
## Method ONE:
## 1. tail -c +3 ylenia3.koa | gzip > temp1.tmp
## 2. printf "\x01\02" | cat - temp1.tmp > temp2.tmp
## 3. tail -c +11 temp2.tmp | head -c -8 > ylenia3.koa.zz
## 4. rm temp1.tmp temp2.tmp
## Method TWO:
## 1. tail -c +3 ylenia3.koa | zlib-flate -compress > ylenia3.koa.zz

.PHONY: all zip d64 prg clean

all: prg zip d64

zip:
	@for f in $(KOA_LIST) ; do \
		echo "Compressing '$$f.koa' ..." ; \
		tail -c +3 $$f.koa | zlib-flate -compress > $$f.koa.zz ; \
	done

prg:
	cl65 -t c64 -O $(OUT_NAME).c -o $(OUT_NAME).prg
	rm -f *.o

d64:
	c1541 -format $(OUT_NAME),AA d64 $(OUT_NAME).d64
	c1541 -attach $(OUT_NAME).d64 -write $(OUT_NAME).prg $(OUT_NAME)
	@for f in $(ZZ_LIST) ; do \
		c1541 -attach $(OUT_NAME).d64 -write $$f.koa.zz %$$f ; \
	done
	c1541 -attach $(OUT_NAME).d64 -dir

clean:
	rm -f *.o *.prg *.sym *.d64 *.koa.zz
