//===-- I8085TargetObjectFile.cpp - I8085 Object Files ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "I8085TargetObjectFile.h"
#include "I8085TargetMachine.h"

#include "llvm/BinaryFormat/ELF.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCSectionELF.h"

#include "I8085.h"

namespace llvm {
void I8085TargetObjectFile::Initialize(MCContext &Ctx, const TargetMachine &TM) {
  Base::Initialize(Ctx, TM);
  ProgmemDataSection =
      Ctx.getELFSection(".progmem.data", ELF::SHT_PROGBITS, ELF::SHF_ALLOC);
  Progmem1DataSection =
      Ctx.getELFSection(".progmem1.data", ELF::SHT_PROGBITS, ELF::SHF_ALLOC);
  Progmem2DataSection =
      Ctx.getELFSection(".progmem2.data", ELF::SHT_PROGBITS, ELF::SHF_ALLOC);
  Progmem3DataSection =
      Ctx.getELFSection(".progmem3.data", ELF::SHT_PROGBITS, ELF::SHF_ALLOC);
  Progmem4DataSection =
      Ctx.getELFSection(".progmem4.data", ELF::SHT_PROGBITS, ELF::SHF_ALLOC);
  Progmem5DataSection =
      Ctx.getELFSection(".progmem5.data", ELF::SHT_PROGBITS, ELF::SHF_ALLOC);
}

MCSection *I8085TargetObjectFile::SelectSectionForGlobal(
    const GlobalObject *GO, SectionKind Kind, const TargetMachine &TM) const {
  // Global values in flash memory are placed in the progmem*.data section
  // unless they already have a user assigned section.
  const auto &I8085TM = static_cast<const I8085TargetMachine &>(TM);
  if (I8085::isProgramMemoryAddress(GO) && !GO->hasSection() &&
      Kind.isReadOnly()) {
    // The I8085 subtarget should support LPM to access section '.progmem*.data'.
    if (!I8085TM.getSubtargetImpl()->hasLPM()) {
      // TODO: Get the global object's location in source file.
      getContext().reportError(
          SMLoc(),
          "Current I8085 subtarget does not support accessing program memory");
      return Base::SelectSectionForGlobal(GO, Kind, TM);
    }
    // The I8085 subtarget should support ELPM to access section
    // '.progmem[1|2|3|4|5].data'.
    if (!I8085TM.getSubtargetImpl()->hasELPM() &&
        I8085::getAddressSpace(GO) != I8085::ProgramMemory) {
      // TODO: Get the global object's location in source file.
      getContext().reportError(SMLoc(),
                               "Current I8085 subtarget does not support "
                               "accessing extended program memory");
      return ProgmemDataSection;
    }
    switch (I8085::getAddressSpace(GO)) {
    case I8085::ProgramMemory: // address space 1
      return ProgmemDataSection;
    case I8085::ProgramMemory1: // address space 2
      return Progmem1DataSection;
    case I8085::ProgramMemory2: // address space 3
      return Progmem2DataSection;
    case I8085::ProgramMemory3: // address space 4
      return Progmem3DataSection;
    case I8085::ProgramMemory4: // address space 5
      return Progmem4DataSection;
    case I8085::ProgramMemory5: // address space 6
      return Progmem5DataSection;
    default:
      llvm_unreachable("unexpected program memory index");
    }
  }

  // Otherwise, we work the same way as ELF.
  return Base::SelectSectionForGlobal(GO, Kind, TM);
}
} // end of namespace llvm
