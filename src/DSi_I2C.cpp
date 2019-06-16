/*
    Copyright 2016-2019 Arisotura

    This file is part of melonDS.

    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#include <stdio.h>
#include <string.h>
#include "DSi.h"
#include "DSi_I2C.h"


namespace DSi_BPTWL
{

u8 Registers[0x100];
u32 CurPos;

bool Init()
{
    return true;
}

void DeInit()
{
}

void Reset()
{
    CurPos = -1;
    memset(Registers, 0x5A, 0x100);

    Registers[0x00] = 0x33; // TODO: support others??
    Registers[0x01] = 0x00;
    Registers[0x02] = 0x50;
    Registers[0x10] = 0x00; // power btn
    Registers[0x11] = 0x00; // reset
    Registers[0x12] = 0x00; // power btn tap
    Registers[0x20] = 0x83; // battery
    Registers[0x21] = 0x07;
    Registers[0x30] = 0x13;
    Registers[0x31] = 0x00; // camera power
    Registers[0x40] = 0x1F; // volume
    Registers[0x41] = 0x04; // backlight
    Registers[0x60] = 0x00;
    Registers[0x61] = 0x01;
    Registers[0x62] = 0x50;
    Registers[0x63] = 0x00;
    Registers[0x70] = 0x00; // boot flag
    Registers[0x71] = 0x00;
    Registers[0x72] = 0x00;
    Registers[0x73] = 0x00;
    Registers[0x74] = 0x00;
    Registers[0x75] = 0x00;
    Registers[0x76] = 0x00;
    Registers[0x77] = 0x00;
    Registers[0x80] = 0x10;
    Registers[0x81] = 0x64;
}

void Start()
{
    printf("BPTWL: start\n");
}

u8 Read(bool last)
{
    if (last)
    {
        CurPos = -1;
        return 0;
    }

    printf("BPTWL: read %02X -> %02X\n", CurPos, Registers[CurPos]);
    return Registers[CurPos++];
}

void Write(u8 val, bool last)
{
    if (last)
    {
        CurPos = -1;
        return;
    }

    if (CurPos == -1)
    {
        CurPos = val;
        printf("BPTWL: reg=%02X\n", val);
        return;
    }

    if (CurPos == 0x11 || CurPos == 0x12 ||
        CurPos == 0x21 ||
        CurPos == 0x30 || CurPos == 0x31 ||
        CurPos == 0x40 || CurPos == 0x31 ||
        CurPos == 0x60 || CurPos == 0x63 ||
        (CurPos >= 0x70 && CurPos <= 0x77) ||
        CurPos == 0x80 || CurPos == 0x81)
    {
        Registers[CurPos] = val;
    }

    printf("BPTWL: write %02X -> %02X\n", CurPos, val);
    CurPos++; // CHECKME
}

}


namespace DSi_I2C
{

u8 Cnt;
u8 Data;

u32 Device;

bool Init()
{
    if (!DSi_BPTWL::Init()) return false;

    return true;
}

void DeInit()
{
    DSi_BPTWL::DeInit();
}

void Reset()
{
    Cnt = 0;
    Data = 0;

    Device = -1;

    DSi_BPTWL::Reset();
}

void WriteCnt(u8 val)
{
    printf("I2C: write CNT %02X\n", val);

    // TODO: check ACK flag
    // TODO: transfer delay
    // TODO: IRQ
    // TODO: check read/write direction

    if (val & (1<<7))
    {
        bool islast = val & (1<<0);

        if (val & (1<<5))
        {
            // read
            val &= 0xF7;

            switch (Device)
            {
            case 0x4A: Data = DSi_BPTWL::Read(islast); break;
            default: Data = 0; break;
            }

            printf("I2C read, device=%02X, cnt=%02X, data=%02X, last=%d\n", Device, val, Data, islast);
        }
        else
        {
            // write
            val &= 0xE7;

            if (val & (1<<1))
            {
                Device = Data & 0xFE;
                printf("I2C: %s start, device=%02X\n", (Data&0x01)?"read":"write", Device);

                switch (Device)
                {
                case 0x4A: DSi_BPTWL::Start(); break;
                }
            }
            else
            {
                printf("I2C write, device=%02X, cnt=%02X, data=%02X, last=%d\n", Device, val, Data, islast);

                switch (Device)
                {
                case 0x4A: DSi_BPTWL::Write(Data, islast); break;
                }
            }

            val |= (1<<4);
        }

        val &= 0x7F;
    }

    Cnt = val;
}

u8 ReadData()
{
    printf("I2C: read the data: %02X\n", Data);
    return Data;
}

void WriteData(u8 val)
{
    Data = val;
}

}
