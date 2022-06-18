//===-- I8085RegisterInfo.td - I8085 Register defs -------------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the I8085 register file
//===----------------------------------------------------------------------===//

// 8-bit General purpose register definition.
class I8085Reg<bits<16> num, string name, list<Register> subregs = [],
             list<string> altNames = []> : RegisterWithSubRegs<name, subregs> {
  field bits<16> Num = num;

  let HWEncoding = num;
  let Namespace = "I8085";
  let SubRegs = subregs;
  let AltNames = altNames;
}

// Subregister indices.
let Namespace = "I8085" in {
  def sub_lo : SubRegIndex<8>;
  def sub_hi : SubRegIndex<8, 8>;
}

let Namespace = "I8085" in { def ptr : RegAltNameIndex; }

//===----------------------------------------------------------------------===//
//  8-bit general purpose registers
//===----------------------------------------------------------------------===//

def A : I8085Reg<7, "A">, DwarfRegNum<[7]>;
def B : I8085Reg<0, "B">, DwarfRegNum<[0]>;
def C : I8085Reg<1, "C">, DwarfRegNum<[1]>;
def D : I8085Reg<2, "D">, DwarfRegNum<[2]>;
def E : I8085Reg<3, "E">, DwarfRegNum<[3]>;
def H : I8085Reg<4, "H">, DwarfRegNum<[4]>;
def L : I8085Reg<5, "L">, DwarfRegNum<[5]>;

def M : I8085Reg<6, "M">, DwarfRegNum<[6]>;

def SP : I8085Reg<7, "SP">, DwarfRegNum<[7]>;

let SubRegIndices = [sub_lo, sub_hi], CoveredBySubRegs = 1 in {
def BC : I8085Reg<0, "C:B", [B, C]>, DwarfRegNum<[0]>;
def DE : I8085Reg<1, "E:D", [D, E]>, DwarfRegNum<[1]>;
def HL : I8085Reg<2, "L:H", [H, L]>, DwarfRegNum<[2]>;
}

def GPRSP : RegisterClass<"I8085", [i16], 8, (add SP)>;

def SREG : I8085Reg<8, "FLAGS">, DwarfRegNum<[8]>;

def CCR : RegisterClass<"I8085", [i8], 8, (add SREG)> {
  let CopyCost = -1; // Don't allow copying of status registers
}

def GR8 : RegisterClass<"I8085", [i8], 8,
                        (
                            // Return value and arguments.
                            add A,B,C,D,E,H,L,M
                            // Scratch registers.
                            )>; 

def GR16 : RegisterClass<"I8085", [i16], 8,
                        (
                            // Return value and arguments.
                            add BC,DE,SP
                            // Scratch registers.
                        )>;                                                      