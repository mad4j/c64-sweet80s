
OUT_NAME=sweet80s

KOA_LIST := ${patsubst %.koa,%,$(wildcard *.koa)}

.PHONY: all d64 prg clean

all: prg d64

prg:
	cl65 -t c64 -O $(OUT_NAME).c -o $(OUT_NAME).prg
	rm -f *.o

d64:
	@echo $(KOA_LIST)
	c1541 -format $(OUT_NAME),AA d64 $(OUT_NAME).d64
	c1541 -attach $(OUT_NAME).d64 -write $(OUT_NAME).prg $(OUT_NAME)
	c1541 -attach $(OUT_NAME).d64 -write 'marta.koa' '!marta'
	c1541 -attach $(OUT_NAME).d64 -write 'ylenia1.koa' '!ylenia1'
	c1541 -attach $(OUT_NAME).d64 -write 'ylenia2.koa' '!ylenia2'
	c1541 -attach $(OUT_NAME).d64 -write 'giada.koa' '!giada'
	c1541 -attach $(OUT_NAME).d64 -write 'marika.koa' '!marika'
	c1541 -attach $(OUT_NAME).d64 -write 'ionela1.koa' '!ionela1'
	c1541 -attach $(OUT_NAME).d64 -dir

clean:
	rm -f *.o *.prg *.sym *.d64
