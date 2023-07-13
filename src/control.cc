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
//  2023-Mar-02  Initial  v0.0.2   ADCL  Add a NOP instruction
//  2023-Mar-11  Initial  v0.0.3   ADCL  Expand the Control Logic to 24 control signals & add R1 controls
//  2023-Mar-19  Initial  v0.0.4   ADCL  Add support for the `MOV R1,<imm16>` instruction
//  2023-Mar-21  Initial  v0.0.5   ADCL  Bug fixes
//  2023-Mar-29  Initial  v0.0.6   ADCL  Add support for the `JMP <imm16>` instruction
//  2023-May-13  Initial  v0.0.7   ADCL  Add support for the `CLC` and `STC` instructions
//  2023-May-22  Initial  v0.0.8   ADCL  Expand the instr to 12 bits; eliminate the direct-wired main bus assert
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

    // bit 5:0 -- assert to Main Bus
    MAIN_BUS_ASSERT_NONE    = (0b000000 << 0) << 0,
    MAIN_BUS_ASSERT_R1      = (0b000001 << 0) << 0,
    MAIN_BUS_ASSERT_R2      = (0b000010 << 0) << 0,
    MAIN_BUS_ASSERT_R3      = (0b000011 << 0) << 0,
    MAIN_BUS_ASSERT_R4      = (0b000100 << 0) << 0,
    MAIN_BUS_ASSERT_R5      = (0b000101 << 0) << 0,
    MAIN_BUS_ASSERT_R6      = (0b000110 << 0) << 0,
    MAIN_BUS_ASSERT_R7      = (0b000111 << 0) << 0,
    MAIN_BUS_ASSERT_R8      = (0b001000 << 0) << 0,
    MAIN_BUS_ASSERT_R9      = (0b001001 << 0) << 0,
    MAIN_BUS_ASSERT_R10     = (0b001010 << 0) << 0,
    MAIN_BUS_ASSERT_R11     = (0b001011 << 0) << 0,
    MAIN_BUS_ASSERT_R12     = (0b001100 << 0) << 0,
    MAIN_BUS_ASSERT_SP      = (0b001101 << 0) << 0,
    MAIN_BUS_ASSERT_RA      = (0b001110 << 0) << 0,
    MAIN_BUS_ASSERT_PC      = (0b001111 << 0) << 0,
    MAIN_BUS_ASSERT_ISP     = (0b010000 << 0) << 0,
    MAIN_BUS_ASSERT_IRA     = (0b010001 << 0) << 0,
    MAIN_BUS_ASSERT_IPC     = (0b010010 << 0) << 0,
    MAIN_BUS_ASSERT_FETCH   = (0b010011 << 0) << 0,
    MAIN_BUS_ASSERT_DEV1    = (0b010100 << 0) << 0,
    MAIN_BUS_ASSERT_DEV2    = (0b010101 << 0) << 0,
    MAIN_BUS_ASSERT_DEV3    = (0b010110 << 0) << 0,
    MAIN_BUS_ASSERT_DEV4    = (0b010111 << 0) << 0,
    MAIN_BUS_ASSERT_DEV5    = (0b011000 << 0) << 0,
    MAIN_BUS_ASSERT_DEV6    = (0b011001 << 0) << 0,
    MAIN_BUS_ASSERT_DEV7    = (0b011010 << 0) << 0,
    MAIN_BUS_ASSERT_DEV8    = (0b011011 << 0) << 0,
    MAIN_BUS_ASSERT_DEV9    = (0b011100 << 0) << 0,
    MAIN_BUS_ASSERT_DEV10   = (0b011101 << 0) << 0,
    MAIN_BUS_ASSERT_ALU     = (0b011110 << 0) << 0,
    MAIN_BUS_ASSERT_MEMORY  = (0b011111 << 0) << 0,

    MAIN_BUS_ASSERT_CTL1    = (0b100100 << 0) << 0,
    MAIN_BUS_ASSERT_CTL2    = (0b100101 << 0) << 0,
    MAIN_BUS_ASSERT_CTL3    = (0b100110 << 0) << 0,
    MAIN_BUS_ASSERT_CTL4    = (0b100111 << 0) << 0,
    MAIN_BUS_ASSERT_CTL5    = (0b101000 << 0) << 0,
    MAIN_BUS_ASSERT_CTL6    = (0b101001 << 0) << 0,
    MAIN_BUS_ASSERT_CTL7    = (0b101010 << 0) << 0,
    MAIN_BUS_ASSERT_CTL8    = (0b101011 << 0) << 0,
    MAIN_BUS_ASSERT_CTL9    = (0b101100 << 0) << 0,
    MAIN_BUS_ASSERT_CTL10   = (0b101101 << 0) << 0,


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

    // bit 3 -- Fetch Assert to Instruction
    INSTRUCTION_ASSERT      = (0b0      << 3) << 8,
    INSTRUCTION_SUPPRESS    = (0b1      << 3) << 8,

    // bit 2 -- Clear the Program Carry
    PGM_CLC                 = (0b1      << 2) << 8,

    // bit 1 -- Set the Program Carry
    PGM_STC                 = (0b1      << 1) << 8,

    // bit 0 -- unused so far


    //---------------------------------------------------

    //
    // == CTRL3
    //    =====

    // bit 7 -- R1 & Latch signal
    R1_AND_LATCH            = (0b1      << 7) << 16,

    // bit 6 -- R2 & Latch signal
    R2_AND_LATCH            = (0b1      << 6) << 16,

    // bit 5 -- PC & Latch signal
    PC_AND_LATCH            = (0b1      << 5) << 16,

    // bits 4:3 -- R2 Load/Inc/Dec
    R2_DO_NOTHING           = (0b00     << 3) << 16,
    R2_LOAD                 = (0b01     << 3) << 16,
    R2_INC                  = (0b10     << 3) << 16,
    R2_DEC                  = (0b11     << 3) << 16,

    // bits 2:0 -- unused so far


    //---------------------------------------------------

    //
    // == Improve code readability
    //    ========================
    FETCH_ASSERT_MAIN       = MAIN_BUS_ASSERT_FETCH | INSTRUCTION_SUPPRESS,
    R1_ASSERT_MAIN          = MAIN_BUS_ASSERT_R1,
    R2_ASSERT_MAIN          = MAIN_BUS_ASSERT_R2,
    R1_LOAD_AND_LATCH       = R1_LOAD | R1_AND_LATCH,
    R2_LOAD_AND_LATCH       = R2_LOAD | R2_AND_LATCH,
    R1_DEC_AND_LATCH        = R1_DEC | R1_AND_LATCH,
    R1_INC_AND_LATCH        = R1_INC | R1_AND_LATCH,
};


//
// -- These are the instructions which will be encoded
//
//    Recall that the instruction word has the following format:
//
//              CCCC IIII IIII IIII
//
//    Where:
//    - CCCC are control flags, used to condition the instruction
//    - IIII IIII IIII is the instruction, encoded in the enum below
//    --------------------------------------------------------------
#include "opcodes.h"


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
    int flags = (loc >> 12) & 0x7;           // top 3 bits of the memory address; flags for augmenting the control signals
    int instr = (loc >>  0) & 0xfff;         // bottom 12 bits for the memory address of the instruction

    const uint64_t nop = ADDR_BUS_1_ASSERT_PC | PC_AND_LATCH | PC_INC | INSTRUCTION_ASSERT | R1_AND_LATCH | R2_AND_LATCH;
    uint64_t out = ADDR_BUS_1_ASSERT_PC | PC_AND_LATCH | PC_INC;

    switch (instr) {
    default:
    case OPCODE_NOP:
        return nop;

    case OPCODE_MOV_R1___16_:
        return out | FETCH_ASSERT_MAIN | R1_LOAD_AND_LATCH;

    case OPCODE_MOV_R2___16_:
        return out | FETCH_ASSERT_MAIN | R2_LOAD_AND_LATCH;

    case OPCODE_MOV_R2_R1:
        return out | R1_ASSERT_MAIN | R2_LOAD_AND_LATCH;

    case OPCODE_MOV_R1_R2:
        return out | R2_ASSERT_MAIN | R1_LOAD_AND_LATCH;

    case OPCODE_JMP___16_:
        return FETCH_ASSERT_MAIN | PC_LOAD | PC_AND_LATCH | INSTRUCTION_SUPPRESS | ADDR_BUS_1_ASSERT_PC;

    case OPCODE_JMP_R1:
        return R1_ASSERT_MAIN | PC_LOAD | PC_AND_LATCH | INSTRUCTION_SUPPRESS | ADDR_BUS_1_ASSERT_PC;

    case OPCODE_JMP_R2:
        return R2_ASSERT_MAIN | PC_LOAD | PC_AND_LATCH | INSTRUCTION_SUPPRESS | ADDR_BUS_1_ASSERT_PC;

    case OPCODE_DECR_R1:
        return out | R1_DEC_AND_LATCH;

    case OPCODE_INCR_R1:
        return out | R1_INC_AND_LATCH;

    case OPCODE_CLC:
        return out | PGM_CLC;

    case OPCODE_STC:
        return out | PGM_STC;
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



