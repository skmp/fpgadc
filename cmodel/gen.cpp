#include <iostream>
#include <stdint.h>
#include <assert.h>
#include <sys/stat.h>

#include <cmath>
#include <string>
#include <algorithm>

#include "shared.h"

static dreamcast_t dc_state;

#include "decoder.inl"

//// mem fns

template<typename T>
bool readMem(dreamcast_t *dc, uint32_t addy, T& data) {
    if ((addy >= 0x8C000000 && addy < 0x8D000000) || addy >= 0x0C000000 && addy < 0x0D000000) {
        data = *reinterpret_cast<T*>(&dc->sys_ram[addy & SYSRAM_MASK]);
        return true;
    } else {
        // return dc->video_ram[addy & VIDEORAM_MASK];

        printf("readMem(%08X)\n", addy);

        return false;
    }
}

template<typename T>
bool writeMem(dreamcast_t *dc, uint32_t addy, T data) {
    if ((addy >= 0x8C000000 && addy < 0x8D000000) || addy >= 0x0C000000 && addy < 0x0D000000) {
        *reinterpret_cast<T*>(&dc->sys_ram[addy & SYSRAM_MASK]) = data;
        return true;
    } else if (addy >= 0xA5000000 && addy < 0xA5800000) {
        *reinterpret_cast<T*>(&dc->video_ram[addy & VIDEORAM_MASK]) = data;
        return true;
    } else {
        // return dc->video_ram[addy & VIDEORAM_MASK];

        printf("writeMem %08X\n", addy);

        return false;
    }
}

std::string findAndReplaceAll(std::string data, std::string toSearch, std::string replaceStr)
{
    // Get the first occurrence
    size_t pos = data.find(toSearch);
 
    // Repeat till end is reached
    while( pos != std::string::npos)
    {
        // Replace this occurrence of Sub String
        data.replace(pos, toSearch.size(), replaceStr);
        // Get the next occurrence from the current position
        pos =data.find(toSearch, pos + replaceStr.size());
    }

    return data;
}
 

std::string file_name(const sh4_opcodelistentry& insn)
{
    char t[512];
    auto s = std::string(insn.diss);
    auto key = insn.key;

    auto s2 = findAndReplaceAll(findAndReplaceAll(s.substr(0, s.find(' ')), ".", "_"), "/", "_"); // yes I googled it *sigh C++*

    sprintf(t, "%s-%04x.inl", s2.c_str(), key);
     
    return std::string(t);
}

void generate_decoder()
{
    printf("\n//decoder.inl\n");

    bool first = true;

    for (auto& insn: opcodes)
    {
        if (!first) printf("else ");
        printf("if ((opcode & 0x%04X) == 0x%04X)\nbegin\n", insn.mask, insn.key);
        printf("`include \"%s\"\n", file_name(insn).c_str());
        printf("end\n");

        first = false;
    }
}

void generate_instruction(const sh4_opcodelistentry& insn)
{
    auto file = file_name(insn);

    printf("\n//%s\n", file.c_str());

    if (insn.dec_op == 0)
    {
        printf("ASSERT_MISSING(\"%s\")\n", insn.diss);
    }
    else
    {
        printf("DECODE_OP(\"%016X\")\n", insn.dec_op);
    }
}

void generate_instructions()
{
    for (auto& insn: opcodes)
    {
        generate_instruction(insn);
    }
}

int main(int argc, char ** argv)
{
    BuildOpcodeTables(&dc_state);
    
    // generate some verilog

    {
        struct stat st = {0};

        if (stat("gen", &st) == -1) {
            mkdir("gen", 0700);
        }
    }

    //

    generate_decoder();

    generate_instructions();

   
    return 0;
}
