add_llvm_component_library(LLVMI8085Disassembler
  I8085Disassembler.cpp

  LINK_COMPONENTS
  MCDisassembler
  I8085Info
  Support

  ADD_TO_COMPONENT
  I8085
)

