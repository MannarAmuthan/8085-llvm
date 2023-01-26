//===-- I8085TargetMachine.h - Define TargetMachine for I8085 -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the I8085 specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_I8085_TARGET_MACHINE_H
#define LLVM_I8085_TARGET_MACHINE_H

#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"

#include "I8085FrameLowering.h"
#include "I8085ISelLowering.h"
#include "I8085InstrInfo.h"
#include "I8085SelectionDAGInfo.h"
#include "I8085Subtarget.h"

namespace llvm {

/// A generic I8085 implementation.
class I8085TargetMachine : public LLVMTargetMachine {
public:
  I8085TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                   StringRef FS, const TargetOptions &Options,
                   std::optional<Reloc::Model> RM, std::optional<CodeModel::Model> CM,
                   CodeGenOpt::Level OL, bool JIT);

  const I8085Subtarget *getSubtargetImpl() const;
  const I8085Subtarget *getSubtargetImpl(const Function &) const override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return this->TLOF.get();
  }

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;

private:
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  I8085Subtarget SubTarget;
};

} // end namespace llvm

#endif // LLVM_I8085_TARGET_MACHINE_H
