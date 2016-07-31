all:fec_rs fec_rdp
.PHONY : clean
fec_rdp:fec_rdp.o
	g++ -g -o fec_rdp fec_rdp.o
fec_rdp.o:fec_rdp.cpp fec_rdp.h
	g++ -g -c  fec_rdp.cpp
fec_rs:fec_rs.o
	g++ -g -o fec_rs fec_rs.o
fec_rs.o:fec_rs.cpp fec_rs.h
	g++ -g -c fec_rs.cpp

clean:
	rm fec_rdp.o fec_rdp fec_rs.o fec_rs