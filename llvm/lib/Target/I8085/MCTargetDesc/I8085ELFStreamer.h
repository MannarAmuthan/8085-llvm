//===----- I8085ELFStreamer.h - I8085 Target Streamer --------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_I8085_ELF_STREAMER_H
#define LLVM_I8085_ELF_STREAMER_H

#include "I8085TargetStreamer.h"

namespace llvm {

/// A target streamer for an I8085 ELF object file.
class I8085ELFStreamer : public I8085TargetStreamer {
public:
  I8085ELFStreamer(MCStreamer &S, const MCSubtargetInfo &STI);

  MCELFStreamer &getStreamer() {
    return static_cast<MCELFStreamer &>(Streamer);
  }
};

} // end namespace llvm

#endif
