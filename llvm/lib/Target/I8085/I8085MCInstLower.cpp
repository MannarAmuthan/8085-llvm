//===-- I8085MCInstLower.cpp - Convert I8085 MachineInstr to an MCInst --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains code to lower I8085 MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "I8085MCInstLower.h"

#include "I8085InstrInfo.h"
#include "MCTargetDesc/I8085MCExpr.h"

#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/ErrorHandling.h"

namespace llvm {

MCOperand I8085MCInstLower::lowerSymbolOperand(const MachineOperand &MO,
                                             MCSymbol *Sym) const {
  unsigned char TF = MO.getTargetFlags();
  const MCExpr *Expr = MCSymbolRefExpr::create(Sym, Ctx);

  bool IsNegated = false;
  if (TF & I8085II::MO_NEG) {
    IsNegated = true;
  }

  if (!MO.isJTI() && MO.getOffset()) {
    Expr = MCBinaryExpr::createAdd(
        Expr, MCConstantExpr::create(MO.getOffset(), Ctx), Ctx);
  }

  bool IsFunction = MO.isGlobal() && isa<Function>(MO.getGlobal());

  if (TF & I8085II::MO_LO) {
    if (IsFunction) {
      // N.B. Should we use _GS fixups here to cope with >128k progmem?
      Expr = I8085MCExpr::create(I8085MCExpr::VK_I8085_PM_LO8, Expr, IsNegated, Ctx);
    } else {
      Expr = I8085MCExpr::create(I8085MCExpr::VK_I8085_LO8, Expr, IsNegated, Ctx);
    }
  } else if (TF & I8085II::MO_HI) {
    if (IsFunction) {
      // N.B. Should we use _GS fixups here to cope with >128k progmem?
      Expr = I8085MCExpr::create(I8085MCExpr::VK_I8085_PM_HI8, Expr, IsNegated, Ctx);
    } else {
      Expr = I8085MCExpr::create(I8085MCExpr::VK_I8085_HI8, Expr, IsNegated, Ctx);
    }
  } else if (TF != 0) {
    llvm_unreachable("Unknown target flag on symbol operand");
  }

  return MCOperand::createExpr(Expr);
}

void I8085MCInstLower::lowerInstruction(const MachineInstr &MI,
                                      MCInst &OutMI) const {
  OutMI.setOpcode(MI.getOpcode());

  for (MachineOperand const &MO : MI.operands()) {
    MCOperand MCOp;

    switch (MO.getType()) {
    default:
      MI.print(errs());
      llvm_unreachable("unknown operand type");
    case MachineOperand::MO_Register:
      // Ignore all implicit register operands.
      if (MO.isImplicit())
        continue;
      MCOp = MCOperand::createReg(MO.getReg());
      break;
    case MachineOperand::MO_Immediate:
      MCOp = MCOperand::createImm(MO.getImm());
      break;
    case MachineOperand::MO_GlobalAddress:
      MCOp = lowerSymbolOperand(MO, Printer.getSymbol(MO.getGlobal()));
      break;
    case MachineOperand::MO_ExternalSymbol:
      MCOp = lowerSymbolOperand(
          MO, Printer.GetExternalSymbolSymbol(MO.getSymbolName()));
      break;
    case MachineOperand::MO_MachineBasicBlock:{
        const llvm::MachineFunction *MF = MO.getMBB()->getParent();
        MCContext &ctx = MF->getContext();
        MCOp = MCOperand::createExpr(
        MCSymbolRefExpr::create(ctx.getOrCreateSymbol("LBB" +Twine(MF->getFunctionNumber()) +Twine(MO.getMBB()->getNumber())), Ctx));
        break;
    }
    case MachineOperand::MO_RegisterMask:
      continue;
    case MachineOperand::MO_BlockAddress:
      MCOp = lowerSymbolOperand(
          MO, Printer.GetBlockAddressSymbol(MO.getBlockAddress()));
      break;
    case MachineOperand::MO_JumpTableIndex:
      MCOp = lowerSymbolOperand(MO, Printer.GetJTISymbol(MO.getIndex()));
      break;
    case MachineOperand::MO_ConstantPoolIndex:
      MCOp = lowerSymbolOperand(MO, Printer.GetCPISymbol(MO.getIndex()));
      break;
    }

    OutMI.addOperand(MCOp);
  }
}

} // end of namespace llvm
