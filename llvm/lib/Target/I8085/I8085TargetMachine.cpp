//===-- I8085TargetMachine.cpp - Define TargetMachine for I8085 ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the I8085 specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#include "I8085TargetMachine.h"

#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/TargetRegistry.h"

#include "I8085.h"
#include "I8085MachineFunctionInfo.h"
#include "I8085TargetObjectFile.h"
#include "MCTargetDesc/I8085MCTargetDesc.h"
#include "TargetInfo/I8085TargetInfo.h"

namespace llvm {

static const char *I8085DataLayout =
    "e-P1-p:16:8-i8:8-i16:8-i32:8-i64:8-f32:8-f64:8-n8-a:8";

/// Processes a CPU name.
static StringRef getCPU(StringRef CPU) {
  if (CPU.empty() || CPU == "generic") {
    return "i8085";
  }

  return CPU;
}

static Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::Static);
}

I8085TargetMachine::I8085TargetMachine(const Target &T, const Triple &TT,
                                   StringRef CPU, StringRef FS,
                                   const TargetOptions &Options,
                                   std::optional<Reloc::Model> RM,
                                   std::optional<CodeModel::Model> CM,
                                   CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(T, I8085DataLayout, TT, getCPU(CPU), FS, Options,
                        getEffectiveRelocModel(RM),
                        getEffectiveCodeModel(CM, CodeModel::Small), OL),
      SubTarget(TT, std::string(getCPU(CPU)), std::string(FS), *this) {
  this->TLOF = std::make_unique<I8085TargetObjectFile>();
  initAsmInfo();
}

namespace {
/// I8085 Code Generator Pass Configuration Options.
class I8085PassConfig : public TargetPassConfig {
public:
  I8085PassConfig(I8085TargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  I8085TargetMachine &getI8085TargetMachine() const {
    return getTM<I8085TargetMachine>();
  }

  void addIRPasses() override;
  bool addInstSelector() override;
  void addPreSched2() override;
  void addPreEmitPass() override;
};
} // namespace

TargetPassConfig *I8085TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new I8085PassConfig(*this, PM);
}

void I8085PassConfig::addIRPasses() {
  // Expand instructions like
  //   %result = shl i32 %n, %amount
  // to a loop so that library calls are avoided.

  TargetPassConfig::addIRPasses();
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeI8085Target() {
  // Register the target.
  RegisterTargetMachine<I8085TargetMachine> X(getTheI8085Target());

  auto &PR = *PassRegistry::getPassRegistry();
  initializeI8085ExpandPseudoPass(PR);
  initializeI8085ExpandPseudo32Pass(PR);
}

const I8085Subtarget *I8085TargetMachine::getSubtargetImpl() const {
  return &SubTarget;
}

const I8085Subtarget *I8085TargetMachine::getSubtargetImpl(const Function &) const {
  return &SubTarget;
}

//===----------------------------------------------------------------------===//
// Pass Pipeline Configuration
//===----------------------------------------------------------------------===//

bool I8085PassConfig::addInstSelector() {
  // Install an instruction selector.
  addPass(createI8085ISelDag(getI8085TargetMachine(), getOptLevel()));
  // Create the frame analyzer pass used by the PEI pass.
  addPass(createI8085FrameAnalyzerPass());

  return false;
}

void I8085PassConfig::addPreSched2() {
  addPass(createI8085ExpandPseudoPass());
  addPass(createI8085ExpandPseudo32Pass());
}

void I8085PassConfig::addPreEmitPass() {
  // Must run branch selection immediately preceding the asm printer.
  addPass(&BranchRelaxationPassID);
}

MachineFunctionInfo *I8085TargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return I8085MachineFunctionInfo::create<I8085MachineFunctionInfo>(Allocator, F,
                                                                STI);
}


} // end of namespace llvm
