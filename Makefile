CXX = g++
CXFLAGS = -std=c++17 -O2 -mavx2
deps = BOBhash32.h Bitmap.h Bloomfilter.h
obj = test.o BOBhash32.o Bitmap.o Bloomfilter.o
program = test.out

$(program): $(obj)
	$(CXX) -o $(program) $(obj) $(CXFLAGS)

BOBhash32.o: BOBhash32.cpp BOBhash32.h
	$(CXX) -c BOBhash32.cpp $(CXFLAGS)

Bitmap.o: Bitmap.cpp Bitmap.h BOBhash32.h
	$(CXX) -c Bitmap.cpp $(CXFLAGS)

Bloomfilter.o: Bloomfilter.cpp Bloomfilter.h BOBhash32.h
	$(CXX) -c Bloomfilter.cpp $(CXFLAGS)

test.o: test.cpp $(deps)
	$(CXX) -c test.cpp $(CXFLAGS)

clean:
	rm -rf $(obj) $(program)