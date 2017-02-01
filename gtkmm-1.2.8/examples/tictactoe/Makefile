include ../examples.conf

all: ttt_test

ttt_test: tictactoe.lo ttt_test.lo
	$(CXXLINK) tictactoe.lo ttt_test.lo $(LIBS)

ttt_test.lo: ttt_test.cc tictactoe.h
	$(CXXCOMPILE) -c $< 

tictactoe.lo: tictactoe.cc tictactoe.h
	$(CXXCOMPILE) -c $< 

clean:
	rm -Rf .libs *.lo core ttt_test

