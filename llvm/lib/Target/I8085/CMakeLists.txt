add_llvm_component_group(I8085)

set(LLVM_TARGET_DEFINITIONS I8085.td)

tablegen(LLVM I8085GenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM I8085GenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM I8085GenCallingConv.inc -gen-callingconv)
tablegen(LLVM I8085GenDAGISel.inc -gen-dag-isel)
tablegen(LLVM I8085GenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM I8085GenInstrInfo.inc -gen-instr-info)
tablegen(LLVM I8085GenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM I8085GenRegisterInfo.inc -gen-register-info)
tablegen(LLVM I8085GenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(I8085CommonTableGen)

add_llvm_target(I8085CodeGen
  I8085AsmPrinter.cpp
  I8085ExpandPseudoInsts.cpp
  I8085FrameLowering.cpp
  I8085InstrInfo.cpp
  I8085ISelDAGToDAG.cpp
  I8085ISelLowering.cpp
  I8085MCInstLower.cpp
  I8085RegisterInfo.cpp
  I8085Subtarget.cpp
  I8085TargetMachine.cpp
  I8085TargetObjectFile.cpp

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  AsmPrinter
  CodeGen
  Core
  MC
  I8085Desc
  I8085Info
  SelectionDAG
  Support
  Target

  ADD_TO_COMPONENT
  I8085
)

add_subdirectory(AsmParser)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
