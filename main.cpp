#include <iostream>
#include <stdint.h>
#include <assert.h>
#include <string.h>
enum Endianness
{
    Big,
    Little
};

enum Mod
{
    Displacement0,
    Displacement8,
    Displacement16,
    RegisterMode
};

enum class RegisterWset
{
    Ax,
    Cx,
    Dx,
    Bx,
    Sp,
    Bp,
    Si,
    Di
};
enum class RegisterWclear
{
    Al,
    Cl,
    Dl,
    Bl,
    Ah,
    Ch,
    Dh,
    Bh
};

enum class RmWclear
{
    Al,
    Cl,
    Dl,
    Bl,
    Ah,
    Ch,
    Dh,
    Bh
};

enum class RmWset
{
    Ax,
    Cx,
    Dx,
    Bx,
    Sp,
    Bp,
    Si,
    Di
};

const char *getRegNameWclear(uint8_t r)
{
    switch (r)
    {
    case 0:
        return "AL";
    case 5:
        return "CH";
    case 1:
        return "CL";
    case 6:
        return "DH";
    case 2:
        return "DL";
    case 7:
        return "BH";
    case 3:
        return "BL";
    case 4:
        return "AH";
    default:
        printf("invalid rgister\n");
        abort();
    }
}

const char *getRegNameWset(uint8_t r)
{
    r &= 0b111;
    switch (r)
    {
    case 0:
        return "AX";
    case 3:
        return "BX";
    case 2:
        return "DX";
    case 6:
        return "SI";
    case 4:
        return "SP";
    case 5:
        return "Bp";
    case 7:
        return "Di";
    case 1:
        return "Cx";
    default:
        printf("invalid rgister %d\n", r);
        abort();
    }
}

const char *getRegName(uint8_t r, int wset)
{
    if (wset)
    {
        return getRegNameWset(r);
    }
    return getRegNameWclear(r);
}

const char *getEAregDisplacement0(uint8_t rm, int16_t imm, char *buf, int buflen)
{
    memset(buf, 0, buflen);
    switch (rm)
    {
    case 0:
        return "[BX + SI]";
    case 1:
        return "[BX + DI]";
    case 2:
        return "[BP + SI]";
    case 3:
        return "[BP + DI]";
    case 4:
        return "[SI]";
    case 5:
        return "[DI]";
    case 6:
    {
        sprintf(buf, "[%d]", imm);
        return buf;
    }
    case 7:
        return "[BX]";
    default:
        printf("Invalid addressing mode: %d\n", rm);
        abort();
    }
}

const char *getEAregDisplacement8(uint8_t rm, int16_t imm, char *buf, int buflen)
{
    memset(buf, 0, buflen);
    switch (rm)
    {
    case 0:
    {
        sprintf(buf, "[BX + SI + %d]", imm);
        return buf;
    }
    case 1:
    {
        sprintf(buf, "[BX + DI + %d]", imm);

        return buf;
    }
    case 2:
    {
        sprintf(buf, "[BP + SI + %d]", imm);

        return buf;
    }
    case 3:
    {
        sprintf(buf, "[BP + DI + %d]", imm);
        return buf;
    }
    case 4:
    {
        sprintf(buf, "[SI + %d]", imm);
        return buf;
    }
    case 5:
    {
        sprintf(buf, "[DI + %d]", imm);
        return buf;
    }
    case 6:
    {
        sprintf(buf, "[BX + %d]", imm);
        return buf;
    }
    case 7:
    {
        sprintf(buf, "[BX + %d]", imm);
        return buf;
    }
    default:
        printf("Invalid addressing mode: %d\n", rm);
        abort();
    }
}

typedef struct
{
    uint32_t word : 1;
    uint32_t reg_is_dest : 1;
    uint32_t opcode : 6;
} __attribute__((packed))
Byte1;

typedef struct
{
    uint32_t rm : 3;
    uint32_t reg : 3;
    Mod mod : 2;
} __attribute__((packed))
Byte2;

struct Reader
{
    FILE *file;

public:
    static bool IsLittleEndian()
    {
        uint16_t x = 1;
        if (reinterpret_cast<uint8_t *>(&x)[0] == 1)
        {
            return true;
        }
        return false;
    }

    template <typename T>
    static T Swap(T val)
    {
        uint8_t *buf = reinterpret_cast<uint8_t *>(&val);
        int half = sizeof(T) / 2;
        for (int i = 0; i < half; i++)
        {
            uint8_t a = buf[i];
            buf[i] = buf[sizeof(T) - i - 1];
            buf[sizeof(T) - i - 1] = a;
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

    uint8_t ReadByte()
    {
        uint8_t buf[1];
        if (std::fread(buf, sizeof(uint8_t), 1, file) == 1)
        {
            return buf[0];
        }

        if(feof(file)) {
            exit(0);
        } 

        std::printf("failed to read byte\n");
        std::exit(1);
    }

    uint16_t ReadWordLE()
    {
        return ReadInt<uint16_t>(Little);
    }

    int8_t ReadSignedByte()
    {
        int8_t buf[1];
        if (std::fread(buf, sizeof(uint8_t), 1, file) == 1)
        {
            return buf[0];
        }

        if(feof(file)) {
            exit(0);
        } 

        std::printf("failed to read byte\n");
        std::exit(1);
    }

    template <typename T>
    T ReadInt(Endianness e)
    {
        uint8_t buf[sizeof(T)];
        if (std::fread(buf, sizeof(uint8_t), sizeof(T), file) == sizeof(T))
        {
            if (e == Endianness::Big)
            {
                if (!IsLittleEndian())
                {
                    return *(T *)buf;
                }
                else
                {
                    return Swap(*(T *)buf);
                }
            }
            else
            {
                if (IsLittleEndian())
                {
                    return *(T *)buf;
                }
                else
                {
                    return Swap(*(T *)buf);
                }
            }
        }
        if(feof(file)) {
            exit(0);
        } 
        std::printf("failed to read int\n");
        std::exit(1);
    }

    void SeekTo(long pos)
    {
        assert(!fseek(file, pos, SEEK_SET));
    }

    void SeekBy(long off)
    {
        assert(!fseek(file, off, SEEK_CUR));
    }

    ~Reader()
    {
        std::fclose(file);
    }
};

enum Instruction
{
    AddRegMem,
    AddAccImm,
    OrRegMem,
    OrAccImm,
    AdcRegMem,
    AdcAccImm,
    SbbRegMem,
    SbbAccImm,
    AndRegMem,
    AndAccImm,
    SubRegMem,
    SubAccImm,
    XorRegMem,
    XorAccImm,
    CmpRegMem,
    CmpAccImm,
    Inc1,
    Inc2,
    Dec1,
    Dec2,
    Push1,
    Push2,
    Pop1,
    Pop2,
    J1 = 0b11100,
    J2,
    J3,
    J4,
    OpImm8,
    TestXchg,
    Mov,
    Mov2,
    Exchg1,
    Exchg2,
    Cbw,
    FOp,
    Mov3,
    String,
    TestStos,
    LodsScas,
    Mov4,
    Mov5,
    Mov6,
    Mov7,
    Ret,
    LesLds,
    Ret2,
    Int,
    Bits,
    Aam,
    Loop = 56,
    IoImm,
    Jmp,
    IoReg,
    Rep,
    Hlt,
    Clc,
    Cld
};

const char *getSegReg(uint8_t s)
{
    switch (s & 0b11)
    {
    case 0:
        return "ES";
        break;
    case 1:
        return "CS";
        break;
    case 2:
        return "SS";
        break;
    case 3:
        return "DS";
        break;
    }
    printf("Unreachable seg reg\n");
    abort();
}

struct InstrDecoder
{
    Reader *buffer;
    char sprintfbuff[50];

public:
    InstrDecoder(Reader *reader) : buffer(reader) {}

    void printMod(Byte1 *b1, Byte2 *b2)
    {
        if (b2->mod == Mod::RegisterMode)
        {
            printf("%s", getRegName(b2->rm, b1->word));
        }
        else if (b2->mod == Mod::Displacement0)
        {
            printf("%s", getEAregDisplacement0(b2->rm,
                                               b2->rm == 6 ? buffer->ReadInt<uint16_t>(Little) : 0, sprintfbuff, 50));
        }
        else if (b2->mod == Mod::Displacement8)
        {
            printf("%s", getEAregDisplacement8(b2->rm, buffer->ReadByte(), sprintfbuff, 50));
        }
        else if (b2->mod == Mod::Displacement16)
        {
            printf("%s", getEAregDisplacement8(b2->rm, buffer->ReadInt<uint16_t>(Little), sprintfbuff, 50));
        }
        else
        {
            printf("unhandled addressing mode\n");
            abort();
        }
    }

    void doStuff() {}

    bool Next()
    {

        uint8_t byte = buffer->ReadByte();
        Byte1 *b1 = reinterpret_cast<Byte1 *>(&byte);
        uint8_t byte2;
        Byte2 *b2;

        switch (b1->opcode)
        {
        case Instruction::Cld:
            byte2 = buffer->ReadByte();
            b2 = reinterpret_cast<Byte2 *>(&byte2);
            switch (byte & 0b11)
            {
            case 0:
                printf("CLD");
                break;
            case 1:
                printf("STD");
                break;
            case 2:
                switch (b2->reg)
                {
                case 0:
                    printf("INC ");
                    printMod(b1, b2);
                    break;
                case 1:
                    printf("INC ");
                    printMod(b1, b2);
                    break;
                default:
                    printf("63 Not used\n");
                    abort();
                    break;
                }
                break;
            case 3:
                switch (b2->reg)
                {
                case 0:
                    printf("INC ");
                    printMod(b1, b2);
                    break;
                case 1:
                    printf("INC ");
                    printMod(b1, b2);
                    break;
                case 2:
                case 3:
                    printf("CALL ");
                    printMod(b1, b2);
                    break;
                case 4:
                case 5:
                    printf("JMP ");
                    printMod(b1, b2);
                    break;
                case 6:
                    printf("PUSH ");
                    printMod(b1, b2);
                    break;
                default:
                    printf("63 Not used\n");
                    abort();
                    break;
                }
                break;
            }
            break;
        case Instruction::Clc:
            switch (byte & 0b11)
            {
            case 0:
                printf("CLC");
                break;
            case 1:
                printf("STC");
                break;
            case 2:
                printf("CLI");
                break;
            case 3:
                printf("STI");
                break;
            }
            break;
        case Instruction::Hlt:
            switch (byte & 0b11)
            {
            case 0:
                printf("HLT");
                break;
            case 1:
                printf("CMC");
                break;
            case 3:
                byte2 = buffer->ReadByte();
                b2 = reinterpret_cast<Byte2 *>(&byte2);
                switch (b2->reg)
                {
                case 0:
                    printf("TEST ");
                    printMod(b1, b2);
                    printf(", %d", buffer->ReadByte());
                    break;
                case 1:
                    printf("HLT Not used\n");
                    abort();
                    break;
                case 2:
                    printf("NOT word ");
                    printMod(b1, b2);
                    break;
                case 3:
                    printf("NEG word ");
                    printMod(b1, b2);
                    break;
                case 4:
                    printf("MUL word ");
                    printMod(b1, b2);
                    break;
                case 5:
                    printf("IMUL word ");
                    printMod(b1, b2);
                    break;
                case 6:
                    printf("DIV word ");
                    printMod(b1, b2);
                    break;
                case 7:
                    printf("IDIV word ");
                    printMod(b1, b2);
                    break;
                }
                break;
            case 2:
                byte2 = buffer->ReadByte();
                b2 = reinterpret_cast<Byte2 *>(&byte2);
                switch (b2->reg)
                {
                case 0:
                    printf("TEST ");
                    printMod(b1, b2);
                    printf(", %d", buffer->ReadByte());
                    break;
                case 1:
                    printf("HLT Not used\n");
                    abort();
                    break;
                case 2:
                    printf("NOT byte ");
                    printMod(b1, b2);
                    break;
                case 3:
                    printf("NEG byte ");
                    printMod(b1, b2);
                    break;
                case 4:
                    printf("MUL byte ");
                    printMod(b1, b2);
                    break;
                case 5:
                    printf("IMUL byte ");
                    printMod(b1, b2);
                    break;
                case 6:
                    printf("DIV byte ");
                    printMod(b1, b2);
                    break;
                case 7:
                    printf("IDIV byte ");
                    printMod(b1, b2);
                    break;
                }
                break;
            }
            break;
        case Instruction::Rep:
            switch (byte & 0b11)
            {
            case 0:
                printf("LOCK");
                break;
            case 1:
                printf("REP Not used\n");
                abort();
                break;
            case 2:
                printf("REPNE");
                break;
            case 3:
                printf("REP");
                break;
            }
            break;
        case Instruction::IoReg:
            switch (byte & 0b11)
            {
            case 0:
                printf("IN AL, DX");
                break;
            case 1:
                printf("IN AX, DX");
                break;
            case 2:
                printf("OUT AL, DX");
                break;
            case 3:
                printf("OUT AX, DX");
                break;
            }
            break;
        case Instruction::Jmp:
            switch (byte & 0b11)
            {
            case 0:
                printf("CALL %d", buffer->ReadInt<int16_t>(Little));
                break;
            case 1:
                printf("JMP %d", buffer->ReadInt<int16_t>(Little));
                break;
            case 2:
                printf("JMP %d:%d", buffer->ReadInt<int16_t>(Little), buffer->ReadInt<int16_t>(Little));
                break;
            case 3:
                printf("JMP %d", buffer->ReadSignedByte());
                break;
            }
            break;
        case Instruction::IoImm:
            switch (byte & 0b11)
            {
            case 0:
                printf("IN AL, %d", buffer->ReadByte());
                break;
            case 1:
                printf("IN AX, %d", buffer->ReadByte());
                break;
            case 2:
                printf("OUT AL, %d", buffer->ReadByte());
                break;
            case 3:
                printf("OUT AX, %d", buffer->ReadByte());
                break;
            }
            break;
        case Instruction::Loop:
            switch (byte & 0b11)
            {
            case 0:
                printf("LOOPNE %d", buffer->ReadSignedByte());
                break;
            case 1:
                printf("LOOPE %d", buffer->ReadSignedByte());
                break;
            case 2:
                printf("LOOP %d", buffer->ReadSignedByte());
                break;
            case 3:
                printf("JCXZ %d", buffer->ReadSignedByte());
                break;
            }
            break;
        case Instruction::Aam:
            switch (byte & 0b11)
            {
            case 0:
                printf("AAM");
                assert(buffer->ReadByte() == 0b1010);
                break;
            case 1:
                printf("AAD");
                assert(buffer->ReadByte() == 0b1010);
                break;
            case 2:
                printf("AAD (Not used)");
                abort();
                break;
            case 3:
                printf("XLAT");
                break;
            }
            break;
        case Instruction::Bits:
            byte2 = buffer->ReadByte();
            b2 = reinterpret_cast<Byte2 *>(&byte2);
            switch (byte & 0b11)
            {
            case 0:
            case 1:
                switch (b2->reg)
                {
                case 0:
                    printf("ROL ");
                    printMod(b1, b2);
                    printf(", 1");
                    break;
                case 1:
                    printf("ROR ");
                    printMod(b1, b2);
                    printf(", 1");
                    break;
                case 2:
                    printf("RCL ");
                    printMod(b1, b2);
                    printf(", 1");
                    break;
                case 3:
                    printf("RCR ");
                    printMod(b1, b2);
                    printf(", 1");
                    break;
                case 4:
                    printf("SHL ");
                    printMod(b1, b2);
                    printf(", 1");
                    break;
                case 5:
                    printf("SHR ");
                    printMod(b1, b2);
                    printf(", 1");
                    break;
                case 6:
                    printf("Bits (Unused 7)\n");
                    abort();
                    break;
                case 7:
                    printf("SAR ");
                    printMod(b1, b2);
                    printf(", 1");
                    break;
                }
                break;
            case 2:
            case 3:
                switch (b2->reg)
                {
                case 0:
                    printf("ROL ");
                    printMod(b1, b2);
                    printf(", CL");
                    break;
                case 1:
                    printf("ROR ");
                    printMod(b1, b2);
                    printf(", CL");
                    break;
                case 2:
                    printf("RCL ");
                    printMod(b1, b2);
                    printf(", CL");
                    break;
                case 3:
                    printf("RCR ");
                    printMod(b1, b2);
                    printf(", CL");
                    break;
                case 4:
                    printf("SHL ");
                    printMod(b1, b2);
                    printf(", CL");
                    break;
                case 5:
                    printf("SHR ");
                    printMod(b1, b2);
                    printf(", CL");
                    break;
                case 6:
                    printf("Bits CL (Unused 7)\n");
                    abort();
                    break;
                case 7:
                    printf("SAR ");
                    printMod(b1, b2);
                    printf(", CL");
                    break;
                }
                break;
            }
            break;
        case Instruction::LesLds:
            byte2 = buffer->ReadByte();
            b2 = reinterpret_cast<Byte2 *>(&byte2);
            switch (byte & 0b11)
            {
            case 0:
                printf("LES ");
                printf("%s, ", getRegNameWset(b2->reg));
                printMod(b1, b2);
                break;
            case 1:
                printf("LDS ");
                printf("%s, ", getRegNameWset(b2->reg));
                printMod(b1, b2);
                break;
            case 2:
                switch (b2->reg)
                {
                case 0:
                    printf("MOV ");
                    printf("%s, %d", getRegNameWclear(b2->reg), buffer->ReadByte());
                    break;
                default:
                    printf("Mov8 (Unused)\n");
                    abort;
                    break;
                }
                break;
            case 3:
                switch (b2->reg)
                {
                case 0:
                    printf("MOV ");
                    printf("%s, %d", getRegNameWset(b2->reg), buffer->ReadWordLE());
                    break;
                default:
                    printf("Mov8 (Unused)\n");
                    abort;
                    break;
                }
                break;
            }
            break;
        case Instruction::Int:
            switch (byte & 0b11)
            {
            case 0:
                printf("INT 3");
                break;
            case 1:
                printf("INT %d", buffer->ReadByte());
                break;
            case 2:
                printf("INTO");
                break;
            case 3:
                printf("IRET");
                break;
            }
            break;
        case Instruction::Ret2:
        case Instruction::Ret:
            switch (byte & 0b11)
            {
            case 0:
            case 1:
                printf("RET (Not used)\n");
                abort();
                break;
            case 2:
                printf("RET %d", buffer->ReadWordLE());
                break;
            case 3:
                printf("RET");
                break;
            }
            break;
        case Instruction::Mov7:
            switch (byte & 0b11)
            {
            case 0:
                printf("MOV SP, %d", buffer->ReadWordLE());
                break;
            case 1:
                printf("MOV BP, %d", buffer->ReadWordLE());
                break;
            case 2:
                printf("MOV SI, %d", buffer->ReadWordLE());
                break;
            case 3:
                printf("MOV DI, %d", buffer->ReadWordLE());
                break;
            }
            break;
        case Instruction::Mov6:
            switch (byte & 0b11)
            {
            case 0:
                printf("MOV AX, %d", buffer->ReadWordLE());
                break;
            case 1:
                printf("MOV CX, %d", buffer->ReadWordLE());
                break;
            case 2:
                printf("MOV DX, %d", buffer->ReadWordLE());
                break;
            case 3:
                printf("MOV BX, %d", buffer->ReadWordLE());
                break;
            }
            break;
        case Instruction::Mov5:
            switch (byte & 0b11)
            {
            case 0:
                printf("MOV AH, %d", buffer->ReadByte());
                break;
            case 1:
                printf("MOV CH, %d", buffer->ReadByte());
                break;
            case 2:
                printf("MOV DH, %d", buffer->ReadByte());
                break;
            case 3:
                printf("MOV BH, %d", buffer->ReadByte());
                break;
            }
            break;
        case Instruction::Mov4:
            switch (byte & 0b11)
            {
            case 0:
                printf("MOV AL, %d", buffer->ReadByte());
                break;
            case 1:
                printf("MOV CL, %d", buffer->ReadByte());
                break;
            case 2:
                printf("MOV DL, %d", buffer->ReadByte());
                break;
            case 3:
                printf("MOV BL, %d", buffer->ReadByte());
                break;
            }
            break;
        case Instruction::LodsScas:
            switch (byte & 0b11)
            {
            case 0:
                printf("LODS byte");
                break;
            case 1:
                printf("LODS word");
                break;
            case 2:
                printf("SCARS byte");
                break;
            case 3:
                printf("SCARS word");
                break;
            }
            break;
        case Instruction::TestStos:
            switch (byte & 0b11)
            {
            case 0:
                printf("TEST AL, %d", buffer->ReadByte());
                break;
            case 1:
                printf("TEST AX, %d", buffer->ReadInt<uint16_t>(Little));
                break;
            case 2:
                printf("STOS byte");
                break;
            case 3:
                printf("STOS word");
                break;
            }
            break;
        case Instruction::String:
            switch (byte & 0b11)
            {
            case 0:
                printf("MOVS byte");
                break;
            case 1:
                printf("MOVS word");
                break;
            case 2:
                printf("CMPS byte");
                break;
            case 3:
                printf("CMPS word");
                break;
            }
            break;
        case Mov3:
            switch (byte & 0b11)
            {
            case 0:
                printf("MOV AL, [%d]", buffer->ReadInt<uint16_t>(Little));
                break;
            case 1:
                printf("MOV AX, [%d]", buffer->ReadInt<uint16_t>(Little));
                break;
            case 2:
                printf("MOV [%d], AL", buffer->ReadInt<uint16_t>(Little));
                break;
            case 3:
                printf("MOV [%d], AX", buffer->ReadInt<uint16_t>(Little));
                break;
            }
            break;
        case Instruction::FOp:
            switch (byte & 0b11)
            {
            case 0:
                printf("PUSHF");
                break;
            case 1:
                printf("POPF");
                break;
            case 2:
                printf("SAHF");
                break;
            case 3:
                printf("LAHF");
                break;
            }
            break;
        case Instruction::Cbw:
            switch (byte & 0b11)
            {
            case 0:
                printf("CBW");
                break;
            case 1:
                printf("CWD");
                break;
            case 2:
                printf("CALL disp[%d] seg[%d]", buffer->ReadInt<int16_t>(Little), buffer->ReadInt<int16_t>(Little));
                break;
            case 3:
                printf("WAIT");
                break;
            }
            break;
        case Instruction::Exchg2:
            switch (byte & 0b11)
            {
            case 0:
                printf("XCHG AX, SP");
                break;
            case 1:
                printf("XCHG AX, BP");
                break;
            case 2:
                printf("XCHG AX, SI");
                break;
            case 3:
                printf("XCHG AX, DI");
                break;
            }
            break;
        case Instruction::Exchg1:
            switch (byte & 0b11)
            {
            case 0:
                printf("NOP");
                break;
            case 1:
                printf("XCHG AX, CX");
                break;
            case 2:
                printf("XCHG AX, DX");
                break;
            case 3:
                printf("XCHG AX, BX");
                break;
            }
            break;
        case Instruction::Mov2:
            byte2 = buffer->ReadByte();
            b2 = reinterpret_cast<Byte2 *>(&byte2);
            switch (byte & 0b11)
            {
            case 0:
                if (b2->reg & 0b100)
                {
                    printf("MOV segreg (not used)\n");
                    abort();
                }
                else
                {
                    printf("MOV ");
                    b1->word = 1;
                    printMod(b1, b2);
                    printf(", %s", getSegReg(b2->reg));
                }
                break;
            case 1:
                printf("LEA %s, ", getRegNameWset(b2->reg));
                printMod(b1, b2);
                break;
            case 2:
                if (b2->reg & 0b100)
                {
                    printf("MOV segreg -> rg (not used)\n");
                    abort();
                }
                else
                {
                    printf("MOV ");
                    b1->word = 1;
                    printf("%s, ", getSegReg(b2->reg));
                    printMod(b1, b2);
                }
                break;
            case 3:
                switch (b2->reg)
                {
                case 0:
                    printf("POP ");
                    printMod(b1, b2);
                    break;
                default:
                    printf("MOV2 (Not used)");
                    abort();
                }
                break;
            default:
                printf("unhandled mov2\n");
                abort();
            }
            break;
        case Instruction::Mov:
            printf("MOV ");
            byte2 = buffer->ReadByte();
            b2 = reinterpret_cast<Byte2 *>(&byte2);
            if (b1->reg_is_dest)
            {
                printf("%s, ", b1->word ? getRegNameWset(b2->reg) : getRegNameWclear(b2->reg));
                printMod(b1, b2);
            }
            else
            {
                printMod(b1, b2);
                printf(", %s", b1->word ? getRegNameWset(b2->reg) : getRegNameWclear(b2->reg));
            }
            break;
        case Instruction::TestXchg:
            printf("TEST ");
            byte2 = buffer->ReadByte();
            b2 = reinterpret_cast<Byte2 *>(&byte2);
            printMod(b1, b2);
            if (b1->word)
            {
                printf(", %s", getRegNameWset(b2->reg));
            }
            else
            {
                printf(", %s", getRegNameWclear(b2->reg));
            }
            break;
        case Instruction::OpImm8:
            byte2 = buffer->ReadByte();
            b2 = reinterpret_cast<Byte2 *>(&byte2);

            switch (byte & 0b11)
            {
            case 0:
                switch (b2->reg)
                {
                case 0:
                    printf("ADD ");
                    break;
                case 1:
                    printf("OR ");
                    break;
                case 2:
                    printf("ADC ");
                    break;
                case 3:
                    printf("SBB ");
                    break;
                case 4:
                    printf("AND ");
                    break;
                case 5:
                    printf("SUB ");
                    break;
                case 6:
                    printf("XOR ");
                    break;
                case 7:
                    printf("CMP ");
                    break;
                default:
                    printf("Invalid immediate instr: %d\n", b2->reg);
                    abort();
                    break;
                }
                printMod(b1, b2);
                printf("%d, ", buffer->ReadByte());
                break;
            case 1:
                switch (b2->reg)
                {
                case 0:
                    printf("ADD ");
                    break;
                case 1:
                    printf("OR ");
                    break;
                case 2:
                    printf("ADC ");
                    break;
                case 3:
                    printf("SBB ");
                    break;
                case 4:
                    printf("AND ");
                    break;
                case 5:
                    printf("SUB ");
                    break;
                case 6:
                    printf("XOR ");
                    break;
                case 7:
                    printf("CMP ");
                    break;
                default:
                    printf("Invalid immediate instr: %d\n", b2->reg);
                    abort();
                    break;
                }
                printMod(b1, b2);
                printf(", %d", buffer->ReadInt<uint16_t>(Big));
                break;
            case 2:
                switch (b2->reg)
                {
                case 0:
                    printf("ADD ");
                    break;
                case 1:
                    printf("OR (Not Used)");
                    abort();
                    break;
                case 2:
                    printf("ADC ");
                    break;
                case 3:
                    printf("SBB ");
                    break;
                case 4:
                    printf("AND  (Not used)");
                    abort();
                    break;
                case 5:
                    printf("SUB ");
                    break;
                case 6:
                    printf("XOR (Not used)");
                    abort();
                    break;
                case 7:
                    printf("CMP ");
                    break;
                default:
                    printf("Invalid immediate instr: %d\n", b2->reg);
                    abort();
                    break;
                }
                printMod(b1, b2);
                printf(", %d", buffer->ReadByte());
                break;
            case 3:
                switch (b2->reg)
                {
                case 0:
                    printf("ADD ");
                    break;
                case 1:
                    printf("OR (Not Used)");
                    abort();
                    break;
                case 2:
                    printf("ADC ");
                    break;
                case 3:
                    printf("SBB ");
                    break;
                case 4:
                    printf("AND  (Not used)");
                    abort();
                    break;
                case 5:
                    printf("SUB ");
                    break;
                case 6:
                    printf("XOR (Not used)");
                    abort();
                    break;
                case 7:
                    printf("CMP ");
                    break;
                default:
                    printf("Invalid immediate instr: %d\n", b2->reg);
                    abort();
                    break;
                }
                printMod(b1, b2);
                printf(", %d", buffer->ReadByte());
                break;
            default:
                printf("unhandled imm: %d\n", b2->reg);
                abort();
            }
            break;
        case Instruction::J4:
            switch (byte & 0b11)
            {
            case 0:
                printf("JL %d", (int8_t)buffer->ReadByte());
                break;
            case 1:
                printf("JNL %d", (int8_t)buffer->ReadByte());
                break;
            case 2:
                printf("JLE %d", (int8_t)buffer->ReadByte());
                break;
            case 3:
                printf("JNLE %d", (int8_t)buffer->ReadByte());
                break;
            }
            break;
        case Instruction::J3:
            switch (byte & 0b11)
            {
            case 0:
                printf("JS %d", (int8_t)buffer->ReadByte());
                break;
            case 1:
                printf("JNS %d", (int8_t)buffer->ReadByte());
                break;
            case 2:
                printf("JP %d", (int8_t)buffer->ReadByte());
                break;
            case 3:
                printf("JNP %d", (int8_t)buffer->ReadByte());
                break;
            }
            break;
        case Instruction::J2:
            switch (byte & 0b11)
            {
            case 0:
                printf("JE %d", (int8_t)buffer->ReadByte());
                break;
            case 1:
                printf("JNE %d", (int8_t)buffer->ReadByte());
                break;
            case 2:
                printf("JBE %d", (int8_t)buffer->ReadByte());
                break;
            case 3:
                printf("JNBE %d", (int8_t)buffer->ReadByte());
                break;
            }
            break;
        case Instruction::J1:
            switch (byte & 0b11)
            {
            case 0:
                printf("JO %d", (int8_t)buffer->ReadByte());
                break;
            case 1:
                printf("JNO %d", (int8_t)buffer->ReadByte());
                break;
            case 2:
                printf("JB %d", (int8_t)buffer->ReadByte());
                break;
            case 3:
                printf("JNB %d", (int8_t)buffer->ReadByte());
                break;
            }
            break;
        case Instruction::Pop1:
            switch (byte & 0b11)
            {
            case 0:
                printf("POP AX");
                break;
            case 1:
                printf("POP CX");
                break;
            case 2:
                printf("POP DX");
                break;
            case 3:
                printf("POP BX");
                break;
            default:
                printf("POP unexpected\n");
                abort();
            }
            break;
        case Instruction::Pop2:
            switch (byte & 0b11)
            {
            case 0:
                printf("POP SP");
                break;
            case 1:
                printf("POP BP");
                break;
            case 2:
                printf("POP SI");
                break;
            case 3:
                printf("POP DI");
                break;
            default:
                printf("POP unexpected\n");
                abort();
            }
            break;
        case Instruction::Push1:
            switch (byte & 0b11)
            {
            case 0:
                printf("PUSH AX");
                break;
            case 1:
                printf("PUSH CX");
                break;
            case 2:
                printf("PUSH DX");
                break;
            case 3:
                printf("PUSH BX");
                break;
            default:
                printf("PUSH unexpected\n");
                abort();
            }
            break;
        case Instruction::Push2:
            switch (byte & 0b11)
            {
            case 0:
                printf("PUSH SP");
                break;
            case 1:
                printf("PUSH BP");
                break;
            case 2:
                printf("PUSH SI");
                break;
            case 3:
                printf("PUSH DI");
                break;
            default:
                printf("PUSH unexpected\n");
                abort();
            }
            break;
        case Instruction::Dec1:
            switch (byte & 0b11)
            {
            case 0:
                printf("DEC AX");
                break;
            case 1:
                printf("DEC CX");
                break;
            case 2:
                printf("DEC DX");
                break;
            case 3:
                printf("DEC BX");
                break;
            default:
                printf("DEC unexpected\n");
                abort();
            }
            break;
        case Instruction::Dec2:
            switch (byte & 0b11)
            {
            case 0:
                printf("DEC SP");
                break;
            case 1:
                printf("DEC BP");
                break;
            case 2:
                printf("DEC SI");
                break;
            case 3:
                printf("DEC DI");
                break;
            default:
                printf("DEC unexpected\n");
                abort();
            }
            break;
        case Instruction::Inc1:
            switch (byte & 0b11)
            {
            case 0:
                printf("INC AX");
                break;
            case 1:
                printf("INC CX");
                break;
            case 2:
                printf("INC DX");
                break;
            case 3:
                printf("INC BX");
                break;
            default:
                printf("INC unexpected\n");
                abort();
            }
            break;
        case Instruction::Inc2:
            switch (byte & 0b11)
            {
            case 0:
                printf("INC SP");
                break;
            case 1:
                printf("INC BP");
                break;
            case 2:
                printf("INC SI");
                break;
            case 3:
                printf("INC DI");
                break;
            default:
                printf("INC unexpected\n");
                abort();
            }
            break;
        case Instruction::CmpRegMem:
            printf("CMP ");

            byte2 = buffer->ReadByte();
            b2 = reinterpret_cast<Byte2 *>(&byte2);
            if (b1->reg_is_dest)
            {
                printf("%s, ", getRegName(b2->reg, b1->word));
                // reg field is the destination register
                printMod(b1, b2);
            }
            else
            {
                // reg field is the source register
                printMod(b1, b2);
                printf(", ");
                printf("%s", getRegName(b2->reg, b1->word));
            }
            break;
        case Instruction::CmpAccImm:
            if (b1->reg_is_dest)
            {
                if (b1->word)
                {
                    printf("AAS");
                }
                else
                {
                    printf("DS:");
                }
            }
            else
            {
                if (b1->word)
                {
                    printf("CMP AX, %d", buffer->ReadInt<uint16_t>(Little));
                }
                else
                {
                    printf("CMP AL, %d", buffer->ReadByte());
                }
            }
            break;
        case AddRegMem:
            printf("ADD ");

            byte2 = buffer->ReadByte();
            b2 = reinterpret_cast<Byte2 *>(&byte2);
            if (b1->reg_is_dest)
            {
                printf("%s, ", getRegName(b2->reg, b1->word));
                // reg field is the destination register
                printMod(b1, b2);
            }
            else
            {
                // reg field is the source register
                printMod(b1, b2);
                printf(", ");
                printf("%s", getRegName(b2->reg, b1->word));
            }
            break;
        case Instruction::AddAccImm:
            if (b1->reg_is_dest)
            {
                if (b1->word)
                {
                    printf("POP ES");
                }
                else
                {
                    printf("PUSH ES");
                }
            }
            else
            {
                if (b1->word)
                {
                    printf("ADD AX, %d", buffer->ReadInt<uint16_t>(Little));
                }
                else
                {
                    printf("ADD AL, %d", buffer->ReadByte());
                }
            }
            break;
        case Instruction::OrRegMem:
            printf("OR ");
            byte2 = buffer->ReadByte();
            b2 = reinterpret_cast<Byte2 *>(&byte2);

            if (b1->reg_is_dest)
            {
                printf("%s, ", b1->word ? getRegNameWset(b2->reg) : getRegNameWclear(b2->reg));
                printMod(b1, b2);
            }
            else
            {
                printMod(b1, b2);
                printf(", ");
                printf("%s", b1->word ? getRegNameWset(b2->reg) : getRegNameWclear(b2->reg));
            }
            break;
        case OrAccImm:
            if (b1->reg_is_dest)
            {
                if (b1->word)
                {
                    printf("Unused Intruction (pop CS): %d\n", byte >> 2);
                    abort();
                }
                else
                {
                    printf("PUSH CS");
                }
            }
            else
            {
                if (b1->word)
                {
                    printf("OR AX, %d", buffer->ReadInt<uint16_t>(Little));
                }
                else
                {
                    printf("OR AL, %d", buffer->ReadByte());
                }
            }
            break;
        case Instruction::AdcRegMem:
            printf("ADC ");
            byte2 = buffer->ReadByte();
            b2 = reinterpret_cast<Byte2 *>(&byte2);
            if (b1->reg_is_dest)
            {
                printf("%s, ", getRegName(b2->reg, b1->word));
                // reg field is the destination register
                printMod(b1, b2);
            }
            else
            {
                // reg field is the source register
                printMod(b1, b2);
                printf(", ");
                printf("%s", getRegName(b2->reg, b1->word));
            }
            break;
        case Instruction::AdcAccImm:
            if (b1->reg_is_dest)
            {
                if (b1->word)
                {
                    printf("POP SS");
                }
                else
                {
                    printf("PUSH SS");
                }
            }
            else
            {
                if (b1->word)
                {
                    printf("ADC AX, %d", buffer->ReadInt<uint16_t>(Little));
                }
                else
                {
                    printf("ADC AL, %d", buffer->ReadByte());
                }
            }
            break;
        case Instruction::SbbRegMem:
            printf("SBB ");
            byte2 = buffer->ReadByte();
            b2 = reinterpret_cast<Byte2 *>(&byte2);
            if (b1->reg_is_dest)
            {
                printf("%s, ", getRegName(b2->reg, b1->word));
                // reg field is the destination register
                printMod(b1, b2);
            }
            else
            {
                // reg field is the source register
                printMod(b1, b2);
                printf(", ");
                printf("%s", getRegName(b2->reg, b1->word));
            }
            break;
        case Instruction::SbbAccImm:
            if (b1->reg_is_dest)
            {
                if (b1->word)
                {
                    printf("POP DS");
                }
                else
                {
                    printf("PUSH DS");
                }
            }
            else
            {
                if (b1->word)
                {
                    printf("SBB AX, %d", buffer->ReadInt<uint16_t>(Little));
                }
                else
                {
                    printf("SBB AL, %d", buffer->ReadByte());
                }
            }
            break;
        case Instruction::AndRegMem:
            printf("AND ");
            byte2 = buffer->ReadByte();
            b2 = reinterpret_cast<Byte2 *>(&byte2);
            if (b1->reg_is_dest)
            {
                printf("%s, ", getRegName(b2->reg, b1->word));
                // reg field is the destination register
                printMod(b1, b2);
            }
            else
            {
                // reg field is the source register
                printMod(b1, b2);
                printf(", ");
                printf("%s", getRegName(b2->reg, b1->word));
            }
            break;
        case Instruction::AndAccImm:
            if (b1->reg_is_dest)
            {
                if (b1->word)
                {
                    printf("DAA");
                }
                else
                {
                    printf("ES:");
                }
            }
            else
            {
                if (b1->word)
                {
                    printf("AND AX, %d", buffer->ReadInt<uint16_t>(Little));
                }
                else
                {
                    printf("AND AL, %d", buffer->ReadByte());
                }
            }
            break;
        case Instruction::SubRegMem:
            printf("SUB ");
            byte2 = buffer->ReadByte();
            b2 = reinterpret_cast<Byte2 *>(&byte2);
            if (b1->reg_is_dest)
            {
                printf("%s, ", getRegName(b2->reg, b1->word));
                // reg field is the destination register
                printMod(b1, b2);
            }
            else
            {
                // reg field is the source register
                printMod(b1, b2);
                printf(", ");
                printf("%s", getRegName(b2->reg, b1->word));
            }
            break;
        case Instruction::SubAccImm:
            if (b1->reg_is_dest)
            {
                if (b1->word)
                {
                    printf("DAS");
                }
                else
                {
                    printf("CS:");
                }
            }
            else
            {
                if (b1->word)
                {
                    printf("SUB AX, %d", buffer->ReadInt<uint16_t>(Little));
                }
                else
                {
                    printf("SUB AL, %d", buffer->ReadByte());
                }
            }
            break;
        case Instruction::XorRegMem:
            printf("XOR ");
            byte2 = buffer->ReadByte();
            b2 = reinterpret_cast<Byte2 *>(&byte2);
            if (b1->reg_is_dest)
            {
                printf("%s, ", getRegName(b2->reg, b1->word));
                // reg field is the destination register
                printMod(b1, b2);
            }
            else
            {
                // reg field is the source register
                printMod(b1, b2);
                printf(", ");
                printf("%s", getRegName(b2->reg, b1->word));
            }
            break;
        case Instruction::XorAccImm:
            if (b1->reg_is_dest)
            {
                if (b1->word)
                {
                    printf("AAA");
                }
                else
                {
                    printf("SS:");
                }
            }
            else
            {
                if (b1->word)
                {
                    printf("SUB AX, %d", buffer->ReadInt<uint16_t>(Little));
                }
                else
                {
                    printf("SUB AL, %d", buffer->ReadByte());
                }
            }
            break;
        default:
            printf("unhandled instruction: %d\n", b1->opcode);
            // fflush(stdout);
            abort();
            break;
        }
        printf("\n");
        return true;
    }

    ~InstrDecoder() {}
};
// const xx = 0b100000;
int main(int argc, char const *argv[])
{
    if(argc != 2) {
        printf("Usage: ./[app] file.bin\n");
        exit(1);
    }
    Reader r(argv[1]);
    InstrDecoder d(&r);
    while (d.Next());
    return 0;
}
