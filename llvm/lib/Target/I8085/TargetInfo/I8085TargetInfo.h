//===-- I8085TargetInfo.h - I8085 Target Implementation -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_I8085_TARGET_INFO_H
#define LLVM_I8085_TARGET_INFO_H

namespace llvm {
class Target;

Target &getTheI8085Target();
} // namespace llvm

#endif // LLVM_I8085_TARGET_INFO_H
