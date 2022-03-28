add_llvm_component_library(LLVMI8085AsmParser
  I8085AsmParser.cpp

  LINK_COMPONENTS
  MC
  MCParser
  I8085Desc
  I8085Info
  Support

  ADD_TO_COMPONENT
  I8085
)
