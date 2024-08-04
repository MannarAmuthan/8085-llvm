#include "I8085ELFStreamer.h"

#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/TargetParser/SubtargetFeature.h"

#include "I8085MCTargetDesc.h"

namespace llvm {

static unsigned getEFlagsForFeatureSet(const FeatureBitset &Features) {
  unsigned EFlags = 0;

  return EFlags;
}

I8085ELFStreamer::I8085ELFStreamer(MCStreamer &S, const MCSubtargetInfo &STI)
    : I8085TargetStreamer(S) {

  ELFObjectWriter &W = getStreamer().getWriter();
  unsigned EFlags = W.getELFHeaderEFlags();

  EFlags |= getEFlagsForFeatureSet(STI.getFeatureBits());

  W.setELFHeaderEFlags(EFlags);
}

} // end namespace llvm
