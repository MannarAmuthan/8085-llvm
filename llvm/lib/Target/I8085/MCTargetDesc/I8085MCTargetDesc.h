//===-- I8085MCTargetDesc.h - I8085 Target Descriptions -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides I8085 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_I8085_MCTARGET_DESC_H
#define LLVM_I8085_MCTARGET_DESC_H

#include "llvm/Support/DataTypes.h"

#include <memory>

namespace llvm {

class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class Target;

MCInstrInfo *createI8085MCInstrInfo();

/// Creates a machine code emitter for I8085.
MCCodeEmitter *createI8085MCCodeEmitter(const MCInstrInfo &MCII,
                                      MCContext &Ctx);

/// Creates an assembly backend for I8085.
MCAsmBackend *createI8085AsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                  const MCRegisterInfo &MRI,
                                  const llvm::MCTargetOptions &TO);

/// Creates an ELF object writer for I8085.
std::unique_ptr<MCObjectTargetWriter> createI8085ELFObjectWriter(uint8_t OSABI);

} // end namespace llvm

#define GET_REGINFO_ENUM
#include "I8085GenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#include "I8085GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "I8085GenSubtargetInfo.inc"

#endif // LLVM_I8085_MCTARGET_DESC_H
