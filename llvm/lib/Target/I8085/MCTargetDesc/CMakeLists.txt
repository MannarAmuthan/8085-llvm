add_llvm_component_library(LLVMI8085Desc
  I8085AsmBackend.cpp
  I8085ELFObjectWriter.cpp
  I8085ELFStreamer.cpp
  I8085InstPrinter.cpp
  I8085MCAsmInfo.cpp
  I8085MCCodeEmitter.cpp
  I8085MCELFStreamer.cpp
  I8085MCExpr.cpp
  I8085MCTargetDesc.cpp
  I8085TargetStreamer.cpp

  LINK_COMPONENTS
  MC
  I8085Info
  Support
  TargetParser

  ADD_TO_COMPONENT
  I8085
)

add_dependencies(LLVMI8085Desc I8085CommonTableGen)

