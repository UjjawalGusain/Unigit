#include "compress.h"
#include "zlib.h"
#include <cassert>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <sstream>

Compressor::Compressor(int chunkSize, int level) {
    this->chunkSize = chunkSize;
    this->level = level;
    this->in.resize(chunkSize);
    this->out.resize(chunkSize);
}

int Compressor::beginDef(std::istream &source, std::ostream &destination) {
    retDef = Z_OK;
    flushDef = Z_NO_FLUSH;

    strmDef.zalloc = Z_NULL;
    strmDef.zfree = Z_NULL;
    strmDef.opaque = Z_NULL;

    this->dest = &destination;
    this->source = &source;

    retDef = deflateInit(&strmDef, this->level);
    if (retDef != Z_OK)
        return retDef;
    return retDef;
}

void Compressor::addChunkDef(const char *data,
                             size_t len, bool isFinal) {
    strmDef.avail_in = static_cast<uInt>(len);
    flushDef = isFinal ? Z_FINISH : Z_NO_FLUSH;
    strmDef.next_in = const_cast<Bytef *>(reinterpret_cast<const Bytef *>(data));

    do {
        strmDef.avail_out = chunkSize;
        strmDef.next_out = out.data();

        retDef = deflate(&strmDef, flushDef);
        assert(retDef != Z_STREAM_ERROR);

        haveDef = chunkSize - strmDef.avail_out;
        dest->write(reinterpret_cast<char *>(out.data()), haveDef);

    } while (strmDef.avail_out == 0);
}

void Compressor::finishDef() {
    flushDef = Z_FINISH;

    do {
        strmDef.avail_out = chunkSize;
        strmDef.next_out = out.data();

        retDef = deflate(&strmDef, flushDef);
        assert(retDef != Z_STREAM_ERROR);

        haveDef = chunkSize - strmDef.avail_out;
        dest->write(reinterpret_cast<char *>(out.data()), haveDef);
    } while (strmDef.avail_out == 0);

    deflateEnd(&strmDef);
}

int Compressor::beginInf(std::istream &source, std::ostream &destination) {
    strmInf.zalloc = Z_NULL;
    strmInf.zfree = Z_NULL;
    strmInf.opaque = Z_NULL;
    strmInf.avail_in = 0;
    strmInf.next_in = Z_NULL;

    this->source = &source;
    this->dest = &destination;

    retInf = inflateInit(&strmInf);
    return retInf;
}

void Compressor::addChunkInf(const char *data, size_t len) {
    strmInf.avail_in = static_cast<uInt>(len);
    strmInf.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(data));

    do {
        strmInf.avail_out = chunkSize;
        strmInf.next_out = out.data();

        retInf = inflate(&strmInf, Z_NO_FLUSH);
        assert(retInf != Z_STREAM_ERROR);

        switch (retInf) {
        case Z_NEED_DICT:
            retInf = Z_DATA_ERROR;
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            inflateEnd(&strmInf);
            return;
        }

        haveInf = chunkSize - strmInf.avail_out;
        dest->write(reinterpret_cast<char *>(out.data()), haveInf);

    } while (strmInf.avail_out == 0);
}

void Compressor::finishInf() {
    inflateEnd(&strmInf);
}

int Compressor::def(std::istream &source, std::ostream &destination) {
    int ret = Z_OK, flush = Z_NO_FLUSH;
    unsigned have;
    z_stream strm;

    std::vector<unsigned char> in(chunkSize);
    std::vector<unsigned char> out(chunkSize);

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    ret = deflateInit(&strm, this->level);
    if (ret != Z_OK)
        return ret;

    do {
        source.read(reinterpret_cast<char *>(in.data()), chunkSize);
        int readCount = source.gcount();
        strm.avail_in = static_cast<uInt>(readCount);

        if (readCount == 0 && !source.eof()) {
            std::cerr << "Source stream gone bad during compressing\n";
            deflateEnd(&strm);
            return Z_ERRNO;
        }

        flush = source.eof() ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in.data();

        do {

            strm.avail_out = chunkSize;
            strm.next_out = out.data();

            ret = deflate(&strm, flush);
            assert(ret != Z_STREAM_ERROR);

            have = chunkSize - strm.avail_out;
            destination.write(reinterpret_cast<char *>(out.data()), have);
            if (!destination) {
                std::cerr << "Destination stream gone bad\n";
                deflateEnd(&strm);
                return Z_ERRNO;
            }

        } while (strm.avail_out == 0);

        assert(strm.avail_in == 0);

    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);

    deflateEnd(&strm);
    return Z_OK;
}

int Compressor::inf(std::istream &source, std::ostream &destination) {
    int ret = Z_OK;
    unsigned have;
    z_stream strm;
    std::vector<unsigned char> in(chunkSize);
    std::vector<unsigned char> out(chunkSize);

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;
    do {
        source.read(reinterpret_cast<char *>(in.data()), chunkSize);
        int readCount = source.gcount();

        strm.avail_in = static_cast<uInt>(readCount);
        if (readCount == 0 && !source.eof()) {
            std::cerr << "Source stream gone bad during decompressing\n";
            inflateEnd(&strm);
            return Z_ERRNO;
        }

        if (strm.avail_in == 0) {
            std::cerr << "strm.avail_in: " << strm.avail_in << "\n";
            break;
        }

        strm.next_in = in.data();

        do {

            strm.avail_out = chunkSize;
            strm.next_out = out.data();

            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);

            switch (ret) {

            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                return ret;
            }

            have = chunkSize - strm.avail_out;
            destination.write(reinterpret_cast<char *>(out.data()), have);
            if (!destination) {
                std::cerr << "Destination stream gone bad during decompression\n";
                inflateEnd(&strm);
                return Z_ERRNO;
            }

        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);

    } while (ret != Z_STREAM_END);
    assert(ret == Z_STREAM_END);
    inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
