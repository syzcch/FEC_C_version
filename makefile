fec_rdp:fec_rdp.o
	g++ -o fec_rdp fec_rdp.o
fec_rdp.o:fec_rdp.cpp fec_rdp.h
	g++ -c  fec_rdp.cpp

clean:
	rm fec_rdp.o fec_rdp