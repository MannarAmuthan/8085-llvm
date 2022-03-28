//===-- I8085TargetStreamer.h - I8085 Target Streamer --------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_I8085_TARGET_STREAMER_H
#define LLVM_I8085_TARGET_STREAMER_H

#include "llvm/MC/MCELFStreamer.h"

namespace llvm {
class MCStreamer;

/// A generic I8085 target output stream.
class I8085TargetStreamer : public MCTargetStreamer {
public:
  explicit I8085TargetStreamer(MCStreamer &S);
};

/// A target streamer for textual I8085 assembly code.
class I8085TargetAsmStreamer : public I8085TargetStreamer {
public:
  explicit I8085TargetAsmStreamer(MCStreamer &S);
};

} // end namespace llvm

#endif // LLVM_I8085_TARGET_STREAMER_H
