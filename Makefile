
OUT_NAME=sweet80s

.PHONY: all d64 prg clean

all: prg d64

prg:
	cl65 -t c64 -O $(OUT_NAME).c -o $(OUT_NAME).prg
	rm -f *.o

d64:
	c1541 -format $(OUT_NAME),AA d64 $(OUT_NAME).d64
	c1541 -attach $(OUT_NAME).d64 -write  $(OUT_NAME).prg $(OUT_NAME)
	c1541 -attach $(OUT_NAME).d64 -write ylenia.koa ylenia
	c1541 -attach $(OUT_NAME).d64 -dir

clean:
	rm -f *.o *.prg *.sym *.d64
