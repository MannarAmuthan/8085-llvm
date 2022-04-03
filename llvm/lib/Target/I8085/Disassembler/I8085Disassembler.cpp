//===- I8085Disassembler.cpp - Disassembler for I8085 ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file is part of the I8085 Disassembler.
//
//===----------------------------------------------------------------------===//

#include "I8085.h"
#include "I8085RegisterInfo.h"
#include "I8085Subtarget.h"
#include "MCTargetDesc/I8085MCTargetDesc.h"
#include "TargetInfo/I8085TargetInfo.h"

#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCFixedLenDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "avr-disassembler"

typedef MCDisassembler::DecodeStatus DecodeStatus;

namespace {

/// A disassembler class for I8085.
class I8085Disassembler : public MCDisassembler {
public:
  I8085Disassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
      : MCDisassembler(STI, Ctx) {}
  virtual ~I8085Disassembler() = default;

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;
};
} // namespace

static MCDisassembler *createI8085Disassembler(const Target &T,
                                             const MCSubtargetInfo &STI,
                                             MCContext &Ctx) {
  return new I8085Disassembler(STI, Ctx);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeI8085Disassembler() {
  // Register the disassembler.
  TargetRegistry::RegisterMCDisassembler(getTheI8085Target(),
                                         createI8085Disassembler);
}

static const uint16_t GPRDecoderTable[] = {
    I8085::A,  I8085::B,  I8085::C,  I8085::D,  I8085::E,  I8085::H,  I8085::L,
    I8085::R0,  I8085::R1,  I8085::R2,  I8085::R3,  I8085::R4,  I8085::R5,  I8085::R6,
    I8085::R7,  I8085::R8,  I8085::R9,  I8085::R10, I8085::R11, I8085::R12, I8085::R13,
    I8085::R14, I8085::R15, I8085::R16, I8085::R17, I8085::R18, I8085::R19, I8085::R20,
    I8085::R21, I8085::R22, I8085::R23, I8085::R24, I8085::R25, I8085::R26, I8085::R27,
    I8085::R28, I8085::R29, I8085::R30, I8085::R31,
};

static DecodeStatus DecodeGPR8RegisterClass(MCInst &Inst, unsigned RegNo,
                                            uint64_t Address,
                                            const MCDisassembler *Decoder) {
  if (RegNo > 31)
    return MCDisassembler::Fail;

  unsigned Register = GPRDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Register));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeLD8RegisterClass(MCInst &Inst, unsigned RegNo,
                                           uint64_t Address,
                                           const MCDisassembler *Decoder) {
  if (RegNo > 15)
    return MCDisassembler::Fail;

  unsigned Register = GPRDecoderTable[RegNo + 16];
  Inst.addOperand(MCOperand::createReg(Register));
  return MCDisassembler::Success;
}

static DecodeStatus DecodePTRREGSRegisterClass(MCInst &Inst, unsigned RegNo,
                                               uint64_t Address,
                                               const MCDisassembler *Decoder) {
  // Note: this function must be defined but does not seem to be called.
  assert(false && "unimplemented: PTRREGS register class");
  return MCDisassembler::Success;
}

static DecodeStatus decodeFIOARr(MCInst &Inst, unsigned Insn, uint64_t Address,
                                 const MCDisassembler *Decoder);

static DecodeStatus decodeFIORdA(MCInst &Inst, unsigned Insn, uint64_t Address,
                                 const MCDisassembler *Decoder);

static DecodeStatus decodeFIOBIT(MCInst &Inst, unsigned Insn, uint64_t Address,
                                 const MCDisassembler *Decoder);

static DecodeStatus decodeCallTarget(MCInst &Inst, unsigned Insn,
                                     uint64_t Address,
                                     const MCDisassembler *Decoder);

static DecodeStatus decodeFRd(MCInst &Inst, unsigned Insn, uint64_t Address,
                              const MCDisassembler *Decoder);

static DecodeStatus decodeFLPMX(MCInst &Inst, unsigned Insn, uint64_t Address,
                                const MCDisassembler *Decoder);

static DecodeStatus decodeFFMULRdRr(MCInst &Inst, unsigned Insn,
                                    uint64_t Address,
                                    const MCDisassembler *Decoder);

static DecodeStatus decodeFMOVWRdRr(MCInst &Inst, unsigned Insn,
                                    uint64_t Address,
                                    const MCDisassembler *Decoder);

static DecodeStatus decodeFWRdK(MCInst &Inst, unsigned Insn, uint64_t Address,
                                const MCDisassembler *Decoder);

static DecodeStatus decodeFMUL2RdRr(MCInst &Inst, unsigned Insn,
                                    uint64_t Address,
                                    const MCDisassembler *Decoder);

#include "I8085GenDisassemblerTables.inc"

static DecodeStatus decodeFIOARr(MCInst &Inst, unsigned Insn, uint64_t Address,
                                 const MCDisassembler *Decoder) {
  unsigned addr = 0;
  addr |= fieldFromInstruction(Insn, 0, 4);
  addr |= fieldFromInstruction(Insn, 9, 2) << 4;
  unsigned reg = fieldFromInstruction(Insn, 4, 5);
  Inst.addOperand(MCOperand::createImm(addr));
  if (DecodeGPR8RegisterClass(Inst, reg, Address, Decoder) ==
      MCDisassembler::Fail)
    return MCDisassembler::Fail;
  return MCDisassembler::Success;
}

static DecodeStatus decodeFIORdA(MCInst &Inst, unsigned Insn, uint64_t Address,
                                 const MCDisassembler *Decoder) {
  unsigned addr = 0;
  addr |= fieldFromInstruction(Insn, 0, 4);
  addr |= fieldFromInstruction(Insn, 9, 2) << 4;
  unsigned reg = fieldFromInstruction(Insn, 4, 5);
  if (DecodeGPR8RegisterClass(Inst, reg, Address, Decoder) ==
      MCDisassembler::Fail)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createImm(addr));
  return MCDisassembler::Success;
}

static DecodeStatus decodeFIOBIT(MCInst &Inst, unsigned Insn, uint64_t Address,
                                 const MCDisassembler *Decoder) {
  unsigned addr = fieldFromInstruction(Insn, 3, 5);
  unsigned b = fieldFromInstruction(Insn, 0, 3);
  Inst.addOperand(MCOperand::createImm(addr));
  Inst.addOperand(MCOperand::createImm(b));
  return MCDisassembler::Success;
}

static DecodeStatus decodeCallTarget(MCInst &Inst, unsigned Field,
                                     uint64_t Address,
                                     const MCDisassembler *Decoder) {
  // Call targets need to be shifted left by one so this needs a custom
  // decoder.
  Inst.addOperand(MCOperand::createImm(Field << 1));
  return MCDisassembler::Success;
}

static DecodeStatus decodeFRd(MCInst &Inst, unsigned Insn, uint64_t Address,
                              const MCDisassembler *Decoder) {
  unsigned d = fieldFromInstruction(Insn, 4, 5);
  if (DecodeGPR8RegisterClass(Inst, d, Address, Decoder) ==
      MCDisassembler::Fail)
    return MCDisassembler::Fail;
  return MCDisassembler::Success;
}

static DecodeStatus decodeFLPMX(MCInst &Inst, unsigned Insn, uint64_t Address,
                                const MCDisassembler *Decoder) {
  if (decodeFRd(Inst, Insn, Address, Decoder) == MCDisassembler::Fail)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(I8085::R31R30));
  return MCDisassembler::Success;
}

static DecodeStatus decodeFFMULRdRr(MCInst &Inst, unsigned Insn,
                                    uint64_t Address,
                                    const MCDisassembler *Decoder) {
  unsigned d = fieldFromInstruction(Insn, 4, 3) + 16;
  unsigned r = fieldFromInstruction(Insn, 0, 3) + 16;
  if (DecodeGPR8RegisterClass(Inst, d, Address, Decoder) ==
      MCDisassembler::Fail)
    return MCDisassembler::Fail;
  if (DecodeGPR8RegisterClass(Inst, r, Address, Decoder) ==
      MCDisassembler::Fail)
    return MCDisassembler::Fail;
  return MCDisassembler::Success;
}

static DecodeStatus decodeFMOVWRdRr(MCInst &Inst, unsigned Insn,
                                    uint64_t Address,
                                    const MCDisassembler *Decoder) {
  unsigned r = fieldFromInstruction(Insn, 4, 4) * 2;
  unsigned d = fieldFromInstruction(Insn, 0, 4) * 2;
  if (DecodeGPR8RegisterClass(Inst, r, Address, Decoder) ==
      MCDisassembler::Fail)
    return MCDisassembler::Fail;
  if (DecodeGPR8RegisterClass(Inst, d, Address, Decoder) ==
      MCDisassembler::Fail)
    return MCDisassembler::Fail;
  return MCDisassembler::Success;
}

static DecodeStatus decodeFWRdK(MCInst &Inst, unsigned Insn, uint64_t Address,
                                const MCDisassembler *Decoder) {
  unsigned d = fieldFromInstruction(Insn, 4, 2) * 2 + 24; // starts at r24:r25
  unsigned k = 0;
  k |= fieldFromInstruction(Insn, 0, 4);
  k |= fieldFromInstruction(Insn, 6, 2) << 4;
  if (DecodeGPR8RegisterClass(Inst, d, Address, Decoder) ==
      MCDisassembler::Fail)
    return MCDisassembler::Fail;
  if (DecodeGPR8RegisterClass(Inst, d, Address, Decoder) ==
      MCDisassembler::Fail)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createImm(k));
  return MCDisassembler::Success;
}

static DecodeStatus decodeFMUL2RdRr(MCInst &Inst, unsigned Insn,
                                    uint64_t Address,
                                    const MCDisassembler *Decoder) {
  unsigned rd = fieldFromInstruction(Insn, 4, 4) + 16;
  unsigned rr = fieldFromInstruction(Insn, 0, 4) + 16;
  if (DecodeGPR8RegisterClass(Inst, rd, Address, Decoder) ==
      MCDisassembler::Fail)
    return MCDisassembler::Fail;
  if (DecodeGPR8RegisterClass(Inst, rr, Address, Decoder) ==
      MCDisassembler::Fail)
    return MCDisassembler::Fail;
  return MCDisassembler::Success;
}

static DecodeStatus readInstruction16(ArrayRef<uint8_t> Bytes, uint64_t Address,
                                      uint64_t &Size, uint32_t &Insn) {
  if (Bytes.size() < 2) {
    Size = 0;
    return MCDisassembler::Fail;
  }

  Size = 2;
  Insn = (Bytes[0] << 0) | (Bytes[1] << 8);

  return MCDisassembler::Success;
}

static DecodeStatus readInstruction32(ArrayRef<uint8_t> Bytes, uint64_t Address,
                                      uint64_t &Size, uint32_t &Insn) {

  if (Bytes.size() < 4) {
    Size = 0;
    return MCDisassembler::Fail;
  }

  Size = 4;
  Insn =
      (Bytes[0] << 16) | (Bytes[1] << 24) | (Bytes[2] << 0) | (Bytes[3] << 8);

  return MCDisassembler::Success;
}

static const uint8_t *getDecoderTable(uint64_t Size) {

  switch (Size) {
  case 2:
    return DecoderTable16;
  case 4:
    return DecoderTable32;
  default:
    llvm_unreachable("instructions must be 16 or 32-bits");
  }
}

DecodeStatus I8085Disassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                             ArrayRef<uint8_t> Bytes,
                                             uint64_t Address,
                                             raw_ostream &CStream) const {
  // uint32_t Insn;

  // DecodeStatus Result;

  // // Try decode a 16-bit instruction.
  // {
  //   Result = readInstruction16(Bytes, Address, Size, Insn);

  //   if (Result == MCDisassembler::Fail)
  //     return MCDisassembler::Fail;

  //   // Try to auto-decode a 16-bit instruction.
  //   Result = decodeInstruction(getDecoderTable(Size), Instr, Insn, Address,
  //                              this, STI);

  //   if (Result != MCDisassembler::Fail)
  //     return Result;
  // }

  // // Try decode a 32-bit instruction.
  // {
  //   Result = readInstruction32(Bytes, Address, Size, Insn);

  //   if (Result == MCDisassembler::Fail)
  //     return MCDisassembler::Fail;

  //   Result = decodeInstruction(getDecoderTable(Size), Instr, Insn, Address,
  //                              this, STI);

  //   if (Result != MCDisassembler::Fail) {
  //     return Result;
  //   }

    return MCDisassembler::Fail;
  
}

typedef DecodeStatus (*DecodeFunc)(MCInst &MI, unsigned insn, uint64_t Address,
                                   const MCDisassembler *Decoder);
