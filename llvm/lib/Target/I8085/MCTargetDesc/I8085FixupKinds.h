//===-- I8085FixupKinds.h - I8085 Specific Fixup Entries ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_I8085_FIXUP_KINDS_H
#define LLVM_I8085_FIXUP_KINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace I8085 {

/// The set of supported fixups.
///
/// Although most of the current fixup types reflect a unique relocation
/// one can have multiple fixup types for a given relocation and thus need
/// to be uniquely named.
///
/// \note This table *must* be in the same order of
///       MCFixupKindInfo Infos[I8085::NumTargetFixupKinds]
///       in `I8085AsmBackend.cpp`.
enum Fixups {
  // /// A 16-bit address I8085 fixup.
  fixup_16 = FirstTargetFixupKind,

  // Marker
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};

namespace fixups {

template <typename T> inline void adjustBranchTarget(T &val) { val >>= 1; }

} // end of namespace fixups
} // namespace I8085
} // namespace llvm

#endif // LLVM_I8085_FIXUP_KINDS_H
