#include <iostream>
#include <stdint.h>
#include <SDL2/SDL.h>
#include <assert.h>
#include "shared.h"
#include <set>

dreamcast_t state;

//// mem fns
template<typename T>
bool readMem(dreamcast_t *dc, uint32_t addy, T& data) {
    data = *reinterpret_cast<T*>(&(dc->memmap[addy >> 24][addy & dc->memmask[addy >> 24]]));
    return true;
}

template<typename T>
bool writeMem(dreamcast_t *dc, uint32_t addy, T data) {
    *reinterpret_cast<T*>(&(dc->memmap[addy >> 24][addy & dc->memmask[addy >> 24]])) = data;
    return true;
}

void initDreamcast(dreamcast_t *dc) {

    memset(dc, 0, sizeof(*dc));

    BuildOpcodeTables(dc);

    memset(dc->pixels, 0, 640 * 480 * sizeof(uint32_t));
    dc->memmap[0x0C] = dc->sys_ram;
    dc->memmask[0x0C] = SYSRAM_MASK;
    dc->memmap[0x8C] = dc->sys_ram;
    dc->memmask[0x8C] = SYSRAM_MASK;
    dc->memmap[0xA5] = dc->video_ram;
    dc->memmask[0xA5] = VIDEORAM_MASK;

    dc->ctx.pc = 0x8C010000;

    FILE* f = fopen("roto.bin", "rb");

    assert(f != NULL && "Failed to load roto.bin");

    auto read = fread(&dc->sys_ram[0x10000], 256, 1, f);

    assert(read == 1);

    fclose(f);

#if DUMP
    for (auto i = 0; i < 256; i+=2) {
        auto offset = 0x10000 + i;

        uint16_t* ptr = reinterpret_cast<uint16_t*>(&dc->sys_ram[offset]);
        uint16_t opcode = *ptr;

        // printf("%08X: %04x %s\n", 0x8C000000 + offset, opcode, OpDesc[opcode]->diss);
        printf("%s\n", dc->OpDesc[opcode]->diss);
    }
#endif
}


void runDreamcast(dreamcast_t *dc) {
    do {
        u16 instr;

        readMem(dc, dc->ctx.pc, instr);

        dc->ctx.pc += 2;
        dc->OpPtr[instr](dc, instr);
    } while (--dc->ctx.remaining_cycles > 0);
}
void runDreamcastStatic(dreamcast_t *dc) {
    for(;;) {
        switch(dc->ctx.pc) {
            L_8C010000:
                dc->ctx.pc = 0x8C010002;
                i1101_nnnn_iiii_iiii(dc, 0xD034);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010004;
                i1110_nnnn_iiii_iiii(dc, 0xE100);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010006;
                i1101_nnnn_iiii_iiii(dc, 0xD232);
                --dc->ctx.remaining_cycles;
            L_8C010006:
                dc->ctx.pc = 0x8C010008;
                i0010_nnnn_mmmm_0010(dc, 0x2012);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01000A;
                i0100_nnnn_0001_0000(dc, 0x4210);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01000C;
                i1000_1111_iiii_iiii_ds(dc, 0x8FFC, i0111_nnnn_iiii_iiii, 0x7004);
                --dc->ctx.remaining_cycles;
                if (__builtin_expect(dc->ctx.remaining_cycles < 0, false)) goto _exitLoop;
                if (dc->ctx.pc == 0x8C010006) goto L_8C010006;
            L_8C01000E:
                dc->ctx.pc = 0x8C010010;
                i1101_nnnn_iiii_iiii(dc, 0xD137);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010012;
                i1101_nnnn_iiii_iiii(dc, 0xD231);
                --dc->ctx.remaining_cycles;
            L_8C010012:
                dc->ctx.pc = 0x8C010014;
                i0110_nnnn_mmmm_0011(dc, 0x6523);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010016;
                i0100_nnnn_0000_1001(dc, 0x4509);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010018;
                i1101_nnnn_iiii_iiii(dc, 0xD330);
                --dc->ctx.remaining_cycles;
            L_8C010018:
                dc->ctx.pc = 0x8C01001A;
                i0110_nnnn_mmmm_0011(dc, 0x6433);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01001C;
                i0100_nnnn_0000_1001(dc, 0x4409);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01001E;
                i0010_nnnn_mmmm_1010(dc, 0x245A);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010020;
                i0010_nnnn_mmmm_0000(dc, 0x2140);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010022;
                i0111_nnnn_iiii_iiii(dc, 0x7101);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010024;
                i0100_nnnn_0001_0000(dc, 0x4310);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010026;
                i1000_1011_iiii_iiii(dc, 0x8BF8);
                --dc->ctx.remaining_cycles;
                if (__builtin_expect(dc->ctx.remaining_cycles < 0, false)) goto _exitLoop;
                if (dc->ctx.pc == 0x8C010018) goto L_8C010018;
            L_8C010026:
                dc->ctx.pc = 0x8C010028;
                i0100_nnnn_0001_0000(dc, 0x4210);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01002A;
                i1000_1011_iiii_iiii(dc, 0x8BF3);
                --dc->ctx.remaining_cycles;
                if (__builtin_expect(dc->ctx.remaining_cycles < 0, false)) goto _exitLoop;
                if (dc->ctx.pc == 0x8C010012) goto L_8C010012;
            L_8C01002A:
                dc->ctx.pc = 0x8C01002C;
                i1010_iiii_iiii_iiii_ds(dc, 0xA000, i0000_0000_0000_1001, 0x0009);
                --dc->ctx.remaining_cycles;
                if (__builtin_expect(dc->ctx.remaining_cycles < 0, false)) goto _exitLoop;
                if (dc->ctx.pc == 0x8C01002E) goto L_8C01002E;
            L_8C01002E:
                dc->ctx.pc = 0x8C010030;
                i1110_nnnn_iiii_iiii(dc, 0xEC01);
                --dc->ctx.remaining_cycles;
            L_8C010030:
                dc->ctx.pc = 0x8C010032;
                i0100_nnnn_0101_1010(dc, 0x4C5A);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010034;
                i1111_nnnn_0010_1101(dc, 0xF42D);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010036;
                i1110_nnnn_iiii_iiii(dc, 0xEE14);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010038;
                i0100_nnnn_0101_1010(dc, 0x4E5A);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01003A;
                i1111_nnnn_0010_1101(dc, 0xF52D);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01003C;
                i1111_nnnn_mmmm_0011(dc, 0xF453);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01003E;
                i1100_0111_iiii_iiii(dc, 0xC72C);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010040;
                i1111_nnnn_mmmm_1000(dc, 0xF608);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010042;
                i1111_nnnn_mmmm_0010(dc, 0xF462);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010044;
                i1111_nnnn_0011_1101(dc, 0xF43D);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010046;
                i1111_nnn0_1111_1101(dc, 0xF0FD);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010048;
                i1111_nnnn_mmmm_1100(dc, 0xF70C);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01004A;
                i1111_nnnn_mmmm_1100(dc, 0xF81C);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01004C;
                i0100_nnnn_0101_1010(dc, 0x4C5A);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01004E;
                i1111_nnnn_0010_1101(dc, 0xF42D);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010050;
                i1110_nnnn_iiii_iiii(dc, 0xEE0A);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010052;
                i0100_nnnn_0101_1010(dc, 0x4E5A);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010054;
                i1111_nnnn_0010_1101(dc, 0xF52D);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010056;
                i1111_nnnn_mmmm_0011(dc, 0xF453);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010058;
                i1111_nnnn_mmmm_0010(dc, 0xF462);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01005A;
                i1111_nnnn_0011_1101(dc, 0xF43D);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01005C;
                i1111_nnn0_1111_1101(dc, 0xF0FD);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01005E;
                i1100_0111_iiii_iiii(dc, 0xC725);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010060;
                i1111_nnnn_mmmm_1000(dc, 0xF208);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010062;
                i1111_nnnn_mmmm_0010(dc, 0xF302);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010064;
                i1100_0111_iiii_iiii(dc, 0xC725);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010066;
                i1111_nnnn_mmmm_1000(dc, 0xF208);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010068;
                i1111_nnnn_mmmm_0000(dc, 0xF320);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01006A;
                i1111_nnnn_mmmm_0010(dc, 0xF732);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01006C;
                i1111_nnnn_mmmm_0010(dc, 0xF832);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01006E;
                i1111_nnnn_0011_1101(dc, 0xF73D);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010070;
                i0000_nnnn_0101_1010(dc, 0x015A);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010072;
                i1111_nnnn_0011_1101(dc, 0xF83D);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010074;
                i0000_nnnn_0101_1010(dc, 0x025A);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010076;
                i1101_nnnn_iiii_iiii(dc, 0xD51A);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010078;
                i0100_nnnn_0000_0001(dc, 0x4501);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01007A;
                i0000_nnnn_mmmm_0111(dc, 0x0517);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01007C;
                i0000_nnnn_0001_1010(dc, 0x031A);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01007E;
                i0110_nnnn_mmmm_1011(dc, 0x633B);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010080;
                i0000_nnnn_mmmm_0111(dc, 0x0527);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010082;
                i0000_nnnn_0001_1010(dc, 0x041A);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010084;
                i0110_nnnn_mmmm_1011(dc, 0x644B);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010086;
                i0110_nnnn_mmmm_0011(dc, 0x6543);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010088;
                i0011_nnnn_mmmm_1000(dc, 0x3438);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01008A;
                i0011_nnnn_mmmm_1100(dc, 0x335C);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01008C;
                i1101_nnnn_iiii_iiii(dc, 0xD017);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01008E;
                i1101_nnnn_iiii_iiii(dc, 0xD515);
                --dc->ctx.remaining_cycles;
            L_8C01008E:
                dc->ctx.pc = 0x8C010090;
                i1101_nnnn_iiii_iiii(dc, 0xD614);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010092;
                i0110_nnnn_mmmm_0011(dc, 0x6733);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010094;
                i0110_nnnn_mmmm_0011(dc, 0x6843);
                --dc->ctx.remaining_cycles;
            L_8C010094:
                dc->ctx.pc = 0x8C010096;
                i1101_nnnn_iiii_iiii(dc, 0xD915);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010098;
                i1110_nnnn_iiii_iiii(dc, 0xEBFF);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01009A;
                i0110_nnnn_mmmm_1100(dc, 0x6BBC);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01009C;
                i0110_nnnn_mmmm_0011(dc, 0x6A73);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C01009E;
                i0100_nnnn_0010_1001(dc, 0x4A29);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100A0;
                i0010_nnnn_mmmm_1001(dc, 0x2AB9);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100A2;
                i0011_nnnn_mmmm_1100(dc, 0x39AC);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100A4;
                i0110_nnnn_mmmm_0011(dc, 0x6A83);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100A6;
                i0100_nnnn_0010_1001(dc, 0x4A29);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100A8;
                i0010_nnnn_mmmm_1001(dc, 0x2AB9);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100AA;
                i0100_nnnn_0001_1000(dc, 0x4A18);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100AC;
                i0011_nnnn_mmmm_1100(dc, 0x39AC);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100AE;
                i0110_nnnn_mmmm_0000(dc, 0x6A90);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100B0;
                i0010_nnnn_mmmm_0001(dc, 0x20A1);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100B2;
                i0111_nnnn_iiii_iiii(dc, 0x7002);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100B4;
                i0011_nnnn_mmmm_1100(dc, 0x371C);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100B6;
                i0011_nnnn_mmmm_1100(dc, 0x382C);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100B8;
                i0100_nnnn_0001_0000(dc, 0x4610);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100BA;
                i1000_1011_iiii_iiii(dc, 0x8BEC);
                --dc->ctx.remaining_cycles;
                if (__builtin_expect(dc->ctx.remaining_cycles < 0, false)) goto _exitLoop;
                if (dc->ctx.pc == 0x8C010094) goto L_8C010094;
            L_8C0100BA:
                dc->ctx.pc = 0x8C0100BC;
                i0011_nnnn_mmmm_1100(dc, 0x332C);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100BE;
                i0011_nnnn_mmmm_1000(dc, 0x3418);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100C0;
                i1101_nnnn_iiii_iiii(dc, 0xDA07);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100C2;
                i0011_nnnn_mmmm_1100(dc, 0x30AC);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100C4;
                i0100_nnnn_0001_0000(dc, 0x4510);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100C6;
                i1000_1011_iiii_iiii(dc, 0x8BE3);
                --dc->ctx.remaining_cycles;
                if (__builtin_expect(dc->ctx.remaining_cycles < 0, false)) goto _exitLoop;
                if (dc->ctx.pc == 0x8C01008E) goto L_8C01008E;
            L_8C0100C6:
                dc->ctx.pc = 0x8C0100C8;
                i0111_nnnn_iiii_iiii(dc, 0x7C01);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100CA;
                i1010_iiii_iiii_iiii_ds(dc, 0xAFB2, i0000_0000_0000_1001, 0x0009);
                --dc->ctx.remaining_cycles;
                if (__builtin_expect(dc->ctx.remaining_cycles < 0, false)) goto _exitLoop;
                if (dc->ctx.pc == 0x8C010030) goto L_8C010030;
            L_8C0100CC:
                dc->ctx.pc = 0x8C0100CE;
                i0000_0000_0000_1001(dc, 0x0009);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100D0;
                i0000_0000_0000_1001(dc, 0x0009);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100D2;
                i0101_nnnn_mmmm_iiii(dc, 0x5800);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100D4;
                i0000_nnnn_0000_0010(dc, 0x0002);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100D6;
                iNotImplemented(dc, 0x0000);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100D8;
                i1010_iiii_iiii_iiii_ds(dc, 0xA500, iNotImplemented, 0x0100);
                --dc->ctx.remaining_cycles;
                if (__builtin_expect(dc->ctx.remaining_cycles < 0, false)) goto _exitLoop;
            L_8C0100DA:
                dc->ctx.pc = 0x8C0100DC;
                iNotImplemented(dc, 0x0000);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100DE;
                iNotImplemented(dc, 0x0280);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100E0;
                iNotImplemented(dc, 0x0000);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100E2;
                iNotImplemented(dc, 0x0140);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100E4;
                iNotImplemented(dc, 0x0000);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100E6;
                iNotImplemented(dc, 0x00F0);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100E8;
                iNotImplemented(dc, 0x0000);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100EA;
                i0101_nnnn_mmmm_iiii(dc, 0x5940);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100EC;
                i1010_iiii_iiii_iiii_ds(dc, 0xA502, iNotImplemented, 0x0000);
                --dc->ctx.remaining_cycles;
                if (__builtin_expect(dc->ctx.remaining_cycles < 0, false)) goto _exitLoop;
            L_8C0100EE:
                dc->ctx.pc = 0x8C0100F0;
                iNotImplemented(dc, 0x0CF0);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100F2;
                i1111_nnnn_mmmm_0011(dc, 0xF983);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100F4;
                i0100_nnnn_0010_0010(dc, 0x4622);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100F6;
                iNotImplemented(dc, 0x0000);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100F8;
                iNotImplemented(dc, 0x4780);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100FA;
                iNotImplemented(dc, 0x0000);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100FC;
                iNotImplemented(dc, 0x4680);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C0100FE;
                i0000_0000_0000_1001(dc, 0x0009);
                --dc->ctx.remaining_cycles;
                dc->ctx.pc = 0x8C010100;
                i0000_0000_0000_1001(dc, 0x0009);
                --dc->ctx.remaining_cycles;
            default:
                for(;;) {
                    u16 instr;
                    readMem(dc, dc->ctx.pc, instr);
                    dc->ctx.pc += 2;
                    dc->OpPtr[instr](dc, instr);
                    switch(dc->ctx.pc) {
                        case 0x8C010000: goto L_8C010000;
                        case 0x8C010006: goto L_8C010006;
                        case 0x8C01000E: goto L_8C01000E;
                        case 0x8C010012: goto L_8C010012;
                        case 0x8C010018: goto L_8C010018;
                        case 0x8C010026: goto L_8C010026;
                        case 0x8C01002A: goto L_8C01002A;
                        case 0x8C01002E: goto L_8C01002E;
                        case 0x8C010030: goto L_8C010030;
                        case 0x8C01008E: goto L_8C01008E;
                        case 0x8C010094: goto L_8C010094;
                        case 0x8C0100BA: goto L_8C0100BA;
                        case 0x8C0100C6: goto L_8C0100C6;
                        case 0x8C0100CC: goto L_8C0100CC;
                        case 0x8C0100DA: goto L_8C0100DA;
                        case 0x8C0100EE: goto L_8C0100EE;
                    }
                }
        }
    }
_exitLoop:
    (void)0;
}
void staticCompileDreamcast(dreamcast_t *dc) {
    std::set<uint32_t> branch_targets;
    branch_targets.insert(0x8C010000);
    for (int pc = 0x8C010000; pc < (0x8C010000 + 256); pc += 2 ) {
        u16 instr;
        readMem(dc, pc, instr);
        if (dc->OpDesc[instr]->is_branch == 1) {
            branch_targets.insert(pc + 2);
        }
        if (dc->OpDesc[instr]->is_branch == 2) {
            branch_targets.insert(pc + 4);
        }
        if (dc->OpDesc[instr]->is_branch) {
            if (dc->OpDesc[instr]->oph != i1010_iiii_iiii_iiii)
                branch_targets.insert(branch_target_s8(instr, pc + 2));
            else
                branch_targets.insert(branch_target_s12(instr, pc + 2));
        }
    }

    printf("void runDreamcastStatic(dreamcast_t *dc) {\n");
    printf("    for(;;) {\n");
    printf("        switch(dc->ctx.pc) {\n");

    for (int pc = 0x8C010000; pc < (0x8C010000 + 256); pc += 2 ) {
        u16 instr;

        if (branch_targets.count(pc)) {
            printf("            L_%08X:\n", pc);
        }
        printf("                dc->ctx.pc = 0x%08X;\n", pc+2);
        readMem(dc, pc, instr);
        if (dc->OpDesc[instr]->is_branch == 2) {
            u16 ds_instr;
            readMem(dc, pc+2, ds_instr);
            printf("                %s_ds(dc, 0x%04X, %s, 0x%04X);\n", dc->OpDesc[instr]->handler_name, instr, dc->OpDesc[ds_instr]->handler_name, ds_instr);
        } else {
            printf("                %s(dc, 0x%04X);\n", dc->OpDesc[instr]->handler_name, instr);
        }
        printf("                --dc->ctx.remaining_cycles;\n");
        if (dc->OpDesc[instr]->is_branch) {
            printf("                if (__builtin_expect(dc->ctx.remaining_cycles < 0, false)) goto _exitLoop;\n");
            if (dc->OpDesc[instr]->is_branch) {
                if (dc->OpDesc[instr]->oph != i1010_iiii_iiii_iiii)
                    printf("                if (dc->ctx.pc == 0x%08X) goto L_%08X;\n", branch_target_s8(instr, pc + 2), branch_target_s8(instr, pc + 2));
                else
                    printf("                if (dc->ctx.pc == 0x%08X) goto L_%08X;\n", branch_target_s12(instr, pc + 2), branch_target_s12(instr, pc + 2));
            }

            if (dc->OpDesc[instr]->is_branch == 2) {
                pc += 2;
            }
        }
    }
    printf("            default:\n");
    printf("                for(;;) {\n");
    printf("                    u16 instr;\n");
    printf("                    readMem(dc, dc->ctx.pc, instr);\n");
    printf("                    dc->ctx.pc += 2;\n");
    printf("                    dc->OpPtr[instr](dc, instr);\n");
    printf("                    switch(dc->ctx.pc) {\n");
    for (int pc = 0x8C010000; pc < (0x8C010000 + 256); pc += 2 ) {
        if (branch_targets.count(pc)) {
            printf("                        case 0x%08X: goto L_%08X;\n", pc, pc);
        }
    }
    printf("                    }\n");
    printf("                }\n");
    printf("        }\n");
    printf("    }\n");
    printf("_exitLoop:\n");
    printf("    (void)0;\n");
    printf("}\n");
}

#ifdef __EMSCRIPTEN__
#include <SDL.h>
#include <emscripten/emscripten.h>
#include <chrono>
#include <cstdio>
#include <algorithm>

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static SDL_Texture* texture = nullptr;
static dreamcast_t* dc = nullptr;
static bool leftMouseButtonDown = false;

static void main_loop()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                emscripten_cancel_main_loop();
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT)
                    leftMouseButtonDown = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                    leftMouseButtonDown = true;
                break;
            case SDL_MOUSEMOTION:
                if (leftMouseButtonDown) {
                    int mouseX = event.motion.x;
                    int mouseY = event.motion.y;
                    if (mouseX >= 0 && mouseX < 640 &&
                        mouseY >= 0 && mouseY < 480) {
                        dc->pixels[mouseY * 640 + mouseX] = 255;
                    }
                }
                break;
        }
    }

    dc->ctx.remaining_cycles += 2000000000;

    auto start = std::chrono::high_resolution_clock::now();
    runDreamcastStatic(dc);  // or runDreamcast(dc)
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    printf("Speed: %f ms per frame @ %.2f mhz\n",
           elapsed.count(),
           2000000000.0 / elapsed.count() / 1000.0);

    memcpy(dc->pixels, dc->video_ram,
           std::min(VIDEORAM_SIZE, 640 * 480 * 2));

    SDL_UpdateTexture(texture, NULL, dc->pixels, 640 * sizeof(u16));

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("SDL2 Pixel Drawing",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              640, 480,
                              SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, 0);
    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_RGB565,
                                SDL_TEXTUREACCESS_STREAMING,
                                640, 480);

    dc = new dreamcast_t();
    initDreamcast(dc);
    staticCompileDreamcast(dc);

    // Register main_loop to be called ~60 times per second
    emscripten_set_main_loop(main_loop, 0, 1);

    return 0; // never reached
}

#else
int main(int argc, char ** argv)
{
    bool leftMouseButtonDown = false;
    bool quit = false;
    SDL_Event event;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window * window = SDL_CreateWindow("SDL2 Pixel Drawing",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);

    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture * texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STATIC, 640, 480);
    

    dreamcast_t* dc = new dreamcast_t();

    initDreamcast(dc);

    staticCompileDreamcast(dc);

    while (!quit)
    {
        dc->ctx.remaining_cycles += 2000000000;
        // get time via c++
        auto start = std::chrono::high_resolution_clock::now();
        //runDreamcast(dc);
        runDreamcastStatic(dc);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        printf("Speed: %f ms per frame @ %.2f mhz\n", elapsed.count(), 2000000000.0 / elapsed.count() / 1000.0);

        memcpy(dc->pixels, dc->video_ram, std::min(VIDEORAM_SIZE,640 * 480 * 2));

        SDL_Delay(1);

        SDL_UpdateTexture(texture, NULL, dc->pixels, 640 * sizeof(u16));

        while(SDL_PollEvent(&event)) {
            switch (event.type)
            {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT)
                        leftMouseButtonDown = false;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT)
                        leftMouseButtonDown = true;
                case SDL_MOUSEMOTION:
                    if (leftMouseButtonDown)
                    {
                        int mouseX = event.motion.x;
                        int mouseY = event.motion.y;
                        dc->pixels[mouseY * 640 + mouseX] = 255;
                    }
                    break;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    delete dc;

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
#endif
