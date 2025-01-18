#pragma once

#include <fstream>


struct BinaryWriter
{
    const std::string filename;
    std::ofstream     outfile;

    explicit
    BinaryWriter(const std::string& filename_)
        : filename{filename_}
        , outfile{filename, std::istream::out | std::ios::binary}
    {}

    ~BinaryWriter()
    {
        if (outfile)
        {
            outfile.close();
        }
    }

    template<typename TValue>
    void write(const TValue& value)
    {
        outfile.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }
};

struct BinaryReader
{
    const std::string filename;
    std::ifstream     infile;

    explicit
    BinaryReader(const std::string& filename_)
            : filename{filename_}
            , infile{filename, std::ios_base::binary}
    {}

    ~BinaryReader()
    {
        if (infile)
        {
            infile.close();
        }
    }

    [[nodiscard]]
    bool isValid() const
    {
        return infile.operator bool();
    }

    template<typename TValue>
    TValue read()
    {
        TValue result = {};
        infile.read(reinterpret_cast<char*>(&result), sizeof(TValue));
        return result;
    }

    template<typename TValue>
    void readInto(TValue& value)
    {
        infile.read(reinterpret_cast<char*>(&value), sizeof(TValue));
    }
};
