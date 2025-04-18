CXX = g++
CXXFLAGS = -Ithirdparty/zlib/include -Ithirdparty/bs_threader/include -Ithirdparty/sha256/include
LDFLAGS = -Lthirdparty/zlib/lib -lz -Lthirdparty/sha256/lib

SRC = src/multithreading/multithreader.cpp \
      src/compression/compress.cpp \
      src/hashing/hasher.cpp \
	  thirdparty/sha256/src/sha256.c \
	  src/blobObject/blobObject.cpp \
	  src/header/header.cpp \
	  src/utils/utils.cpp \
	  main.cpp

OUT = compress.exe

compress: $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) $(LDFLAGS) -o $(OUT)

run: compress
	./$(OUT)

clean:
	$(RM) $(OUT)
