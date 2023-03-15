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
//  2023-Mar-11  Initial  v0.0.2   ADCL  Expand the Control Logic to 24 control signals & add R1 controls
//
//===================================================================================================================


#include <cstdint>
#include <stdint.h>
#include <stdio.h>


//
// -- These are the different control signals which can be enabled on the PROM
//    ------------------------------------------------------------------------
enum {
    //
    // == CTRL1
    //    =====

    // bits 7:6 -- assert to Address Bus 1
    ADDR_BUS_1_ASSERT_PC    = (0b00     << 6) << 0,
    ADDR_BUS_1_ASSERT_RA    = (0b01     << 6) << 0,
    ADDR_BUS_1_ASSERT_INTPC = (0b10     << 6) << 0,
    ADDR_BUS_1_ASSERT_INTRA = (0b11     << 6) << 0,

    // bit 5 -- group assert to Main Bus
    MAIN_BUS_GROUP_0        = (0b0      << 5) << 0,
    MAIN_BUS_GROUP_1        = (0b0      << 5) << 0,

    // bits 4:0 -- unused so far


    //---------------------------------------------------

    //
    // == CTRL2
    //    =====

    // bits 7:6 -- R1 Load/Inc/Dec
    R1_DO_NOTHING           = (0b00     << 6) << 8,
    R1_LOAD                 = (0b01     << 6) << 8,
    R1_INC                  = (0b10     << 6) << 8,
    R1_DEC                  = (0b11     << 6) << 8,

    // bits 5:4 -- PC Load/Inc/Dec
    PC_DO_NOTHING           = (0b00     << 4) << 8,
    PC_LOAD                 = (0b01     << 4) << 8,
    PC_INC                  = (0b10     << 4) << 8,
    PC_DEC                  = (0b11     << 4) << 8,

    // bits 3:0 -- unused so far


    //---------------------------------------------------

    //
    // == CTRL3
    //    =====

    // bit 7 -- R1 & Latch signal
    R1_AND_LATCH            = (0b0      << 7) << 16,

    // bit 6 -- PC & Latch signal
    PC_AND_LATCH            = (0b0      << 6) << 16,

    // bits 5:0 -- unused so far
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
uint64_t promBuffer [PROM_SIZE];


//
// -- Break the prom location down to the flags and instruction portions
//    and determine the control lines for each possible combination
//    ------------------------------------------------------------------
uint64_t GenerateControlSignals(int loc)
{
    int flags = (loc >> 24) & 0x0f;         // top 4 bits
    int instr = (loc >> 4) & 0xff;          // middle 8 bits
    // -- bottom 4 bits are inconsequential here

    const uint64_t nop = ADDR_BUS_1_ASSERT_PC | PC_AND_LATCH | PC_INC;

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
        promBuffer[i] = GenerateControlSignals(i);
    }


    // -- Open each output file in turn
    FILE *of1 = fopen("ctrl1.bin", "w");
    if (!of1) perror("Unable to open ctrl1.bin");

    FILE *of2 = fopen("ctrl2.bin", "w");
    if (!of2) perror("Unable to open ctrl2.bin");

    FILE *of3 = fopen("ctrl3.bin", "w");
    if (!of3) perror("Unable to open ctrl3.bin");


    // -- write each EEPROM
    for (int i = 0; i < PROM_SIZE; i ++) {
        uint8_t byte1 = (promBuffer[i] >>  0) & 0xff;
        uint8_t byte2 = (promBuffer[i] >>  8) & 0xff;
        uint8_t byte3 = (promBuffer[i] >> 16) & 0xff;

        fwrite(&byte1, 1, sizeof(uint8_t), of1);
        fwrite(&byte2, 1, sizeof(uint8_t), of2);
        fwrite(&byte3, 1, sizeof(uint8_t), of3);
    }

    // -- Flush the buffers -- just to be sure
    fflush(of1);
    fflush(of2);
    fflush(of3);


    // -- close the files
    fclose(of1);
    fclose(of2);
    fclose(of3);
}



