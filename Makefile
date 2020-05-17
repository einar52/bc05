
#CC = cc -static
CC = gcc
L =  
B = /usr/local/bin
S = /sil/bin
CFLAGS =  -O  -I/usr/local/include
CFLAGS =  -O -static  -I/usr/sil/include
CFLAGS = -g -I/usr/sil/include 
W = getwd.o getwdata.o silread.o bitcomprl.o 
#V = getwdata.o silread.o bitcomprl.o time.o evlib.o util.o shift55_8.o isdata.o
V = getwdata.o silread.o bitcomprl.o time.o evlib.o util.o shift55_8.o isbcdata.o linux.o 
HH = silread.h bitcomprl.h evlib.h util.h 
SS = bctool.c getwdata.c silread.c bitcomprl.c time.c evlib.c util.c mbctool.c ${HH}
#LL = /usr/local/lib/libbc.a
#LL = /usr/local/lib/libbc.so
LL = libbc.so

tt : libbc.a
t : mbctool
#	mbctool c < /heim/eik/www/axel/040000.mt > j.mbc
	mbctool u < j.mbc  > j.mt
mbctool : mbctool.o libbc.a
	cc -o mbctool mbctool.o libbc.a -lm
P=bcplot.o getwdata.o evlib.o shift55_8.o bitcomprl.o time.o silread.o linux.o  util.o ../../safer/filter/detfil.o
bcplot : $P
	filtutil -i1 -p 5 5.5 -p 11.5  8  -p 7.9 14 -z 1 -A 16 -w 5-12.filt
# Sparc
#	f77 -o bcplot bcplot.o detfil.o shift55_8.o -lbc -lplois -lgd0
# Intel x86
#	f77 -o bcplot bcplot.o detfil.o shift55_8.o -lbc -lploisg -lgd -lf2c
	gcc -o bcplot $P ../../safer/plois_f2c/libploisg.a  -lgd -lf2c -lm
5-15.filt :
jjjj: getwd 
	getwd
lib : ${LL}
${LL} : $V
#	cc -static -o ${LL} $V
#	cc -shared -o ${LL} $V
	gcc -shared -o ${LL} $V
libbc.a : $V
	ar rcs libbc.a $V
getwdata.o speed.o : getwdata.h
getwd : ${LL} getwd.o 
	f77 -o getwd getwd.o -lbc  -lplois -lgd0
jjj : getwdatat
	getwdatat
Z = getwdata.o silread.o bitcomprl.o  util.o time.o evlib.o
getwdatat : getwdatat.o libbc.a  Makefile
	cc   -o getwdatat getwdatat.o libbc.a  -lm
rb :	testbc 
	testbc < test/testbc.dat | wc
testbc : testbc.o Makefile $Z
	cc -o testbc testbc.o $Z -lmalloc -lm
xxx : bctool
	bctool -l 6 -i 980922 
install : bctool
	cp bctool /sil/bin
bc2ah.o bc2sil.o bctool.o sil2bc.o silread.o siltest.o : silread.h
#bctool  : bctool.o ${LL}
#	cc $L -o bctool bctool.o -lbc -lm
bctool  : bctool.o libbc.a
	cc -static -o bctool bctool.o libbc.a -lm
H = bc2ah.o util.o bitcomprl.o silread.o
testh : bc2ah
	bc2ah -b /eq1 -l 9 -f 000311.bc
bc2ah : $H
	cc -o bc2ah $L $H -lah -lm -lnsl -lev
J = bc2sil.o silread.o bitcomprl.o 
bc2sil : $J
	cc -o bc2sil $J -lev -lm
T = siltest.o silread.o bitcomprl.o
siltest : $T
	cc -o siltest $T  -lev -lm
	siltest
silread.o now2sil : silread.h
test : sil2bc
	sil2bc -l 6 -d 32 -p /eq0/1998 -m 2
speedt : speed
	time speed < /eq1/1998/sep/22/lib.mag
speed : speed.o
	cc -o speed speed.o libbc.a
N = sil2bc.o silread.o util.o bitcomprl.o
sil2bc : $N
	cc $L -o sil2bc $N -lev -lm 
U = bcutil.o bitcomprl.o
bcutil : $U
	cc $L -o bcutil $U  -lm
test_shift : shift55_8
	shift55_8
shift55_8 : 
	cc -c -DTEST shift55_8.c
	f77 -o shift55_8  shift55_8.o -lplois -lgd0
	rm shift55_8.o
clean :
	rm bcutil *.o sil2bc siltest core* ved.tar bctool speed a.out shift55_8 libbc.so
ved.tar : ${SS} Makefile
	tar chvf ved.tar ${SS} mktime.c

SSJSRC=libbc.a(getwdata.o) libbc.a(silread.o) libbc.a(bitcomprl.o) libbc.a(time.o) \
	libbc.a(evlib.o) libbc.a( util.o) libbc.a(shift55_8.o) libbc.a(isdata.o) libbc.a(linux.o)

#	libbc.a(evlib.o) libbc.a( util.o) libbc.a(shift55_8.o) libbc.a(isdata.o)

bc:	$(SSJSRC)
	
