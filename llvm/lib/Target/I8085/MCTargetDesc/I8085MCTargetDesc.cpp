//===-- I8085MCTargetDesc.cpp - I8085 Target Descriptions ---------------------===//
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

#include "I8085MCTargetDesc.h"
#include "I8085ELFStreamer.h"
#include "I8085InstPrinter.h"
#include "I8085MCAsmInfo.h"
#include "I8085MCELFStreamer.h"
#include "I8085TargetStreamer.h"
#include "TargetInfo/I8085TargetInfo.h"

#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "I8085GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "I8085GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "I8085GenRegisterInfo.inc"

using namespace llvm;

MCInstrInfo *llvm::createI8085MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitI8085MCInstrInfo(X);

  return X;
}

static MCRegisterInfo *createI8085MCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitI8085MCRegisterInfo(X, 0);

  return X;
}

static MCSubtargetInfo *createI8085MCSubtargetInfo(const Triple &TT,
                                                 StringRef CPU, StringRef FS) {
  return createI8085MCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCInstPrinter *createI8085MCInstPrinter(const Triple &T,
                                             unsigned SyntaxVariant,
                                             const MCAsmInfo &MAI,
                                             const MCInstrInfo &MII,
                                             const MCRegisterInfo &MRI) {
  if (SyntaxVariant == 0) {
    return new I8085InstPrinter(MAI, MII, MRI);
  }

  return nullptr;
}

static MCStreamer *createMCStreamer(const Triple &T, MCContext &Context,
                                    std::unique_ptr<MCAsmBackend> &&MAB,
                                    std::unique_ptr<MCObjectWriter> &&OW,
                                    std::unique_ptr<MCCodeEmitter> &&Emitter,
                                    bool RelaxAll) {
  return createELFStreamer(Context, std::move(MAB), std::move(OW),
                           std::move(Emitter), RelaxAll);
}

static MCTargetStreamer *
createI8085ObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  return new I8085ELFStreamer(S, STI);
}

static MCTargetStreamer *createMCAsmTargetStreamer(MCStreamer &S,
                                                   formatted_raw_ostream &OS,
                                                   MCInstPrinter *InstPrint,
                                                   bool isVerboseAsm) {
  return new I8085TargetAsmStreamer(S);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeI8085TargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfo<I8085MCAsmInfo> X(getTheI8085Target());

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(getTheI8085Target(), createI8085MCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(getTheI8085Target(), createI8085MCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(getTheI8085Target(),
                                          createI8085MCSubtargetInfo);

  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(getTheI8085Target(),
                                        createI8085MCInstPrinter);

  // Register the MC Code Emitter
  TargetRegistry::RegisterMCCodeEmitter(getTheI8085Target(),
                                        createI8085MCCodeEmitter);

  // Register the obj streamer
  TargetRegistry::RegisterELFStreamer(getTheI8085Target(), createMCStreamer);

  // Register the obj target streamer.
  TargetRegistry::RegisterObjectTargetStreamer(getTheI8085Target(),
                                               createI8085ObjectTargetStreamer);

  // Register the asm target streamer.
  TargetRegistry::RegisterAsmTargetStreamer(getTheI8085Target(),
                                            createMCAsmTargetStreamer);

  // Register the asm backend (as little endian).
  TargetRegistry::RegisterMCAsmBackend(getTheI8085Target(), createI8085AsmBackend);
}
