#ifndef HEADER_H
#define HEADER_H

class HeaderBuilder {
    public:
        static std::string buildBlobHeader(size_t fileSize, std::string fileType);
        static std::string getHeaderFromCompressedObject(const std::filesystem::path& compressedFilePath);
    };

#endif