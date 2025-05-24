#ifndef COMPRESS_H
#define COMPRESS_H

#include "zlib.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

class Compressor {
public:
    int chunkSize, level, retDef, flushDef, retInf;
    unsigned haveDef, haveInf;
    z_stream strmDef, strmInf;
    std::vector<unsigned char> in, out;
    std::istream *source;
    std::ostream *dest;

    explicit Compressor(int chunkSize, int level);

    int beginDef(std::ifstream &, std::ostream &);
    void addChunkDef(const char *data, size_t len, bool isFinal = false);
    void finishDef();

    int beginInf(std::istream &, std::ostream &);
    void addChunkInf(const char *data, size_t len);
    void finishInf();

    int def(std::ifstream &source, std::ofstream &destination);
    int inf(std::ifstream &source, std::ofstream &destination);
};

#endif
