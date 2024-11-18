#include <iostream>
#include <stdint.h>
#include<assert.h>

enum Endianness{
    Big, Little
};

struct Reader
{
    FILE *file;

public:
    static bool IsLittleEndian() {
        uint16_t x = 1;
        if(reinterpret_cast<uint8_t *>(&x)[0] == 1) {
            return true;
        }
        return false;
    }

    template <typename T>
    static T Swap(T val) {
        uint8_t *buf = reinterpret_cast<uint8_t *>(&val);
        int half = sizeof(T)/2;
        for(int i = 0; i < half; i++) {
            uint8_t a = buf[i];
            buf[i] = buf[sizeof(T)-i-1];
            buf[sizeof(T)-i-1] = a;
        }
        return val;
    }

    Reader(std::FILE *f) : file(f) {}
    
    Reader(const char *file_name)
    {
        std::FILE *file = std::fopen(file_name, "rb");
        if (!file)
        {
            std::printf("failed to open binary file");
            std::exit(1);
        }

        this->file = file;
    }

    uint8_t ReadByte() {
        uint8_t buf[1];
        if(std::feof(file)) {
            return 0;
        }
        if(std::fread(buf, sizeof(uint8_t), 1, file) == 1) {
            return buf[0];
        }

        std::printf("failed to read byte\n");
        std::exit(1);
    }

    template <typename T>
    T ReadInt(Endianness e) {
        uint8_t buf[sizeof(T)];
        if(std::feof(file)) {
            return 0;
        }
        if(std::fread(buf, sizeof(uint8_t), sizeof(T), file) == sizeof(T)) {
            if(e == Endianness::Big) {
                if(!IsLittleEndian()) {
                    return *(T*)buf;
                } else {
                    return Swap(*(T*)buf);
                }
            } else {
                if(IsLittleEndian()) {
                    return *(T*)buf;
                } else {
                    return Swap(*(T*)buf);
                }
            }
        }

        std::printf("failed to read byte\n");
        std::exit(1);
    }

    void SeekTo(long pos) {
        assert(!fseek(file, pos, SEEK_SET));
    }

    void SeekBy(long off) {
        assert(!fseek(file, off, SEEK_CUR));
    }

    ~Reader() {
        std::fclose(file);
    }
};

int main(int argc, char const *argv[])
{
    Reader r("bin");

    //char a[4] = {'h','e','l','p'};

    //uint32_t a = Reader::Swap<uint32_t>(300);
    //uint32_t b = Reader::Swap<uint32_t>(a);



    //assert(b == 300);

    uint16_t big = r.ReadInt<uint16_t>(Endianness::Big);
    r.SeekTo(0);
    uint16_t small = r.ReadInt<uint16_t>(Endianness::Little);

    printf("small: %d, big: %d, sbig: %d\n", small,big, Reader::Swap(big));


    if(Reader::IsLittleEndian()) {
        printf("Little endian arch\n");

    }

    return 0;
}
