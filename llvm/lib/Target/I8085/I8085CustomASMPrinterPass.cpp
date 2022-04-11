#define DEBUG_TYPE "machinecount"
#include "I8085.h"
#include "llvm/Pass.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

namespace {
class MachineCountPass : public MachineFunctionPass {
public:
 static char ID;
  MachineCountPass() : MachineFunctionPass(ID) {}

  virtual bool runOnMachineFunction(MachineFunction &MF) {
    unsigned num_instr = 0;
    
    // errs() << MF.getName() << "\n";

    for (MachineFunction::iterator I = MF.begin(), E = MF.end();I != E; ++I) {
          MachineBasicBlock& BB = *I;
      for (MachineBasicBlock::iterator BBI = I->begin(),BBE = I->end(); BBI != BBE; ++BBI) {
          MachineInstr& BBIn = *BBI;
        //   switch(BBIn.getOpcode()){
        //       default:
                //  errs() << BBIn.getOpcode() << "\n";
                //  BBIn.dump();
        //   }
          ++num_instr;
      }
    }
    return false;
  }
};
}

FunctionPass *llvm::createI8085CustomASMPrinterPass() {
  return new MachineCountPass();
}

char MachineCountPass::ID = 0;
static RegisterPass<MachineCountPass> X("machinecount", "Machine Count Pass");