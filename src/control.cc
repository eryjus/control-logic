//===================================================================================================================
//  control.cc -- Generate the control logic for the 16-Bit Computer From Scratch
//
//  This file will generate the control logic for the 16-Bit Computer From Scratch.  This is intended to be a
//  temporary solution for the breadboard incarnation.  When we get to moving this to PCB, a different solution
//  will be used (as in, not EEPROM).
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2023-Feb-24  Initial  v0.0.1   ADCL  Initial version
//
//===================================================================================================================


#include <cstdint>
#include <stdint.h>
#include <stdio.h>


//
// -- These are the different control signals which can be enabled on the PROM
//    ------------------------------------------------------------------------
enum {
    // bits 7:5 -- assert to Address Bus 1
    ADDR_BUS_1_ASSERT_PC    = (0b000    << 5),

    // bits 4:2 -- AND LATCH
    AND_LATCH_PC            = (0b000    << 2),

    // bits 1:0 -- PC Load/Inc/Dec
    PC_DO_NOTHING           = (0b00     << 0),
    PC_LOAD                 = (0b01     << 0),
    PC_INC                  = (0b10     << 0),
    PC_DEC                  = (0b11     << 0),
};


//
// -- These are the instructions which will be encoded
//    ------------------------------------------------
enum {
    NOP                     = 0x00,
};


//
// -- the size of the eeprom
//    ----------------------
const int PROM_SIZE = 1024 * 32;         // we are using 32KB EEPROM


//
// -- this eeprom buffer(s)
//    ---------------------
uint8_t promBuffer [PROM_SIZE];


//
// -- Break the prom location down to the flags and instruction portions
//    and determine the control lines for each possible combination
//    ------------------------------------------------------------------
uint64_t GenerateControlSignals(int loc)
{
    int flags = (loc >> 24) & 0x0f;         // top 4 bits
    int instr = (loc >> 4) & 0xff;          // middle 8 bits
    // -- bottom 4 bits are inconsequential here

    const uint64_t nop = ADDR_BUS_1_ASSERT_PC | AND_LATCH_PC | PC_INC;

    switch (instr) {
    case NOP:
        return nop;

    default:
        return nop;
    }
}



//
// -- Main entry point
//    ----------------
int main(void)
{
    for (int i = 0; i < PROM_SIZE; i ++) {
        uint64_t controlWord = GenerateControlSignals(i);

        promBuffer[i] = controlWord & 0xff;
    }

    FILE *of1 = fopen("ctrl1.bin", "w");
    if (!of1) perror("Unable to open ctrl1.bin");

    fwrite(promBuffer, sizeof(promBuffer), sizeof(uint8_t), of1);
    fflush(of1);
    fclose(of1);
}



