// #ifndef COMPRESS_H
// #define COMPRESS_H

// #include "zlib.h"
// #include <cassert>
// #include <fstream>
// #include <iostream>
// #include <string>
// #include <thread>
// #include <vector>

// class Compressor {
// public:
//     int chunkSize, level, retDef, flushDef, retInf;
//     unsigned haveDef, haveInf;
//     z_stream strmDef, strmInf;
//     std::vector<unsigned char> in, out;
//     std::istream *source;
//     std::ostream *dest;

//     explicit Compressor(int chunkSize, int level);

//     int beginDef(std::ifstream &, std::ostream &);
//     void addChunkDef(const char *data, size_t len, bool isFinal = false);
//     void finishDef();

//     int beginInf(std::istream &, std::ostream &);
//     void addChunkInf(const char *data, size_t len);
//     void finishInf();

//     int def(std::ifstream &source, std::ofstream &destination);
//     int inf(std::ifstream &source, std::ofstream &destination);
// };

// #endif
#ifndef COMPRESS_H
#define COMPRESS_H

#include "zlib.h"
#include <cassert>
#include <string>
#include <vector>

class Compressor {
public:
    int chunkSize, level, retDef, flushDef, retInf;
    unsigned haveDef, haveInf;
    z_stream strmDef, strmInf;
    std::vector<unsigned char> in, out;
    std::ostream *dest;
    std::istream *source;

    explicit Compressor(int chunkSize, int level);

    int beginDef(std::istream &source, std::ostream &destination);
    void addChunkDef(const char *data, size_t len, bool isFinal);
    void finishDef();

    int beginInf(std::istream &source, std::ostream &destination);
    void addChunkInf(const char *data, size_t len);
    void finishInf();

    int def(std::istream &source, std::ostream &destination);
    int inf(std::istream &source, std::ostream &destination);
};

#endif
