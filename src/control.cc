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
    MAIN_BUS_GROUP_1        = (0b1      << 5) << 0,

    // bit 4 -- Fetch Assert to Instruction
    INSTRUCTION_ASSERT      = (0b0      << 4) << 0,
    INSTRUCTION_SUPPRESS    = (0b1      << 4) << 0,

    // bit 3 -- Clear the Program Carry
    PGM_CLC                 = (0b1      << 3) << 0,

    // bit 2 -- Set the Program Carry
    PGM_STC                 = (0b1      << 2) << 0,

    // bits 1:0 -- unused so far


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
    R1_AND_LATCH            = (0b1      << 7) << 16,

    // bit 6 -- PC & Latch signal
    PC_AND_LATCH            = (0b1      << 6) << 16,

    // bits 5:0 -- unused so far


    //---------------------------------------------------

    //
    // == Improve code readability
    //    ========================
    FETCH_ASSERT_MAIN       = MAIN_BUS_GROUP_1 | INSTRUCTION_SUPPRESS,
    PC_ASSERT_MAIN          = MAIN_BUS_GROUP_0,
    R1_LOAD_AND_LATCH       = R1_LOAD | R1_AND_LATCH,
};


//
// -- These are the instructions which will be encoded
//
//    Recall that the instruction word has the following format:
// 
//              CCCC IIII IIII MMMM
//
//    Where:
//    - CCCC are control flags, used to condition the instruction
//    - IIII IIII is the instruction, encoded in the enum below
//    - MMMM is the main bus assert, hard-wired into the control module
//    -----------------------------------------------------------------
enum {
    NOP                     = 0x00,
    MOV_R1_IMMED            = 0x01,
    CLC                     = 0x02,
    STC                     = 0x03,
    MOV_R1_PC               = 0x04,

    JMP_IMMED               = 0xff,
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
    int flags = (loc >> 16) & 0x7f;         // top 7 bits of the memory address; flags for augmenting the control signals
    int instr = (loc >> 0) & 0xff;          // bottom 8 bits for the memory address
    // -- bottom 4 bits are inconsequential here

    const uint64_t nop = ADDR_BUS_1_ASSERT_PC | PC_AND_LATCH | PC_INC | INSTRUCTION_ASSERT;
    uint64_t out = ADDR_BUS_1_ASSERT_PC | PC_AND_LATCH | PC_INC;

    switch (instr) {
    case NOP:
        return nop;

    case MOV_R1_IMMED:
        return out | FETCH_ASSERT_MAIN | R1_LOAD_AND_LATCH;

    case JMP_IMMED:
        return FETCH_ASSERT_MAIN | PC_LOAD | PC_AND_LATCH | INSTRUCTION_SUPPRESS | ADDR_BUS_1_ASSERT_PC;      

    case CLC:
        return out | PGM_CLC;

    case STC:
        return out | PGM_STC;

    case MOV_R1_PC:
        return out | PC_ASSERT_MAIN | R1_LOAD_AND_LATCH;

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



