all:fec_rs fec_rdp fec_star
.PHONY : clean
fec_rdp:fec_rdp.o
	g++ -g -o fec_rdp fec_rdp.o
fec_rdp.o:fec_rdp.cpp fec_rdp.h
	g++ -g -c  fec_rdp.cpp
fec_rs:fec_rs.o
	g++ -g -o fec_rs fec_rs.o
fec_rs.o:fec_rs.cpp fec_rs.h
	g++ -g -c fec_rs.cpp
fec_star:fec_star.o
	g++ -g -o fec_star fec_star.o
fec_star.o:fec_star.cpp fec_star.h
	g++ -g -c fec_star.cpp

clean:
	rm fec_rdp.o fec_rdp fec_rs.o fec_rs fec_star.o fec_star