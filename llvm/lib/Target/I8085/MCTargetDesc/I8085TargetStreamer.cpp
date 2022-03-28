//===-- I8085TargetStreamer.cpp - I8085 Target Streamer Methods ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides I8085 specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "I8085TargetStreamer.h"

#include "llvm/MC/MCContext.h"

namespace llvm {

I8085TargetStreamer::I8085TargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

I8085TargetAsmStreamer::I8085TargetAsmStreamer(MCStreamer &S)
    : I8085TargetStreamer(S) {}

} // end namespace llvm
