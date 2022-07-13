let Constraints = "$src = $rd", Defs=[SREG]  in {

  def SUB_32
      : I8085Pseudo<(outs GR32: $rd),
              (ins GR32: $src, GR32: $rr),
              "SUB_32\t$rd, $rr",
              [(set i32: $rd, (sub i32: $src, i32: $rr))]>;

  def ADD_32
      : I8085Pseudo<(outs GR32: $rd),
              (ins GR32: $src, GR32: $rr),
              "ADD_32\t$rd, $rr",
              [(set i32: $rd, (add i32: $src, i32: $rr))]>;

  def ADDI_32
      : I8085Pseudo<(outs GR32: $rd),
              (ins GR32: $src, i32imm: $k),
              "ADDI_32\t$rd, $k",
              [(set i32: $rd, (add i32: $src, imm: $k))]>;

 
  def SUBI_32
      : I8085Pseudo<(outs GR32: $rd),
              (ins GR32: $src, i32imm: $k),
              "SUBI_32\t$rd, $k",
              [(set i32: $rd, (sub i32: $src, imm: $k))]>; 


  def AND_32
      : I8085Pseudo<(outs GR32: $rd),
              (ins GR32: $src, GR32: $rr),
              "AND_32\t$rd, $rr",
              [(set i32: $rd, (and i32: $src, i32: $rr))]>; 

  def XOR_32
      : I8085Pseudo<(outs GR32: $rd),
              (ins GR32: $src, GR32: $rr),
              "XOR_32\t$rd, $rr",
              [(set i32: $rd, (xor i32: $src, i32: $rr))]>;

  def OR_32
      : I8085Pseudo<(outs GR32: $rd),
              (ins GR32: $src, GR32: $rr),
              "OR_32\t$rd, $rr",
              [(set i32: $rd, (or i32: $src, i32: $rr))]>;                          
  
  def ANDI_32
      : I8085Pseudo<(outs GR32: $rd),
              (ins GR32: $src, i32imm: $k),
              "ANDI_32\t$rd, $k",
              [(set i32: $rd, (and i32: $src, imm: $k))]>; 

  def XORI_32
      : I8085Pseudo<(outs GR32: $rd),
              (ins GR32: $src, i32imm: $k),
              "XORI_32\t$rd, $k",
              [(set i32: $rd, (xor i32: $src, imm: $k))]>; 

  def ORI_32
      : I8085Pseudo<(outs GR32: $rd),
              (ins GR32: $src, i32imm: $k),
              "ORI_32\t$rd, $k",
              [(set i32: $rd, (or i32: $src, imm: $k))]>; 

  def SHL_32
      : I8085InserterPseudo<(outs GR32: $rd),
              (ins GR32: $src, GR8: $rr),
              "SHL_32\t$rd, $rr",
              [(set i32: $rd, (shl i32: $src, i8: $rr))]>;

  def SRA_32
      : I8085InserterPseudo<(outs GR32: $rd),
              (ins GR32: $src, GR8: $rr),
              "SRA_32\t$rd, $rr",
              [(set i32: $rd, (sra i32: $src, i8: $rr))]>;                                   
}


let Constraints = "$rs = $rd", Defs=[SREG]  in {
  let Defs=[SREG,A], Uses=[A] in def RL_32 : I8085Pseudo<(outs GR32: $rd), (ins GR32: $rs), "RL_32 $rs", []>;
  let Defs=[SREG,A], Uses=[A] in def RR_32 : I8085Pseudo<(outs GR32: $rd), (ins GR32: $rs), "RR_32 $rs", []>;
}

let Uses=[SP] in {
def LOAD_32 : I8085Pseudo<(outs GR32: $dst),
                       (ins i32imm: $src),
                       "LOAD_32 $dst, $src", [(set i32
                                             : $dst, imm
                                             : $src)]>; 

def STORE_32 : I8085Pseudo<(outs),
                            (ins mspi: $dst, GR32: $src),
                            "STORE_32 $dst, $src", [(store i32: $src, addr: $dst)]>;
                            

def LOAD_32_WITH_ADDR : I8085Pseudo<(outs GR32: $dst),
                       (ins mspi: $src),
                       "LOAD $dst, $src", [(set i32: $dst, (load addr: $src))]>;    


def STORE_32_AT_OFFSET_WITH_SP : I8085Pseudo<(outs),
                       (ins GR32: $src, i16imm: $offset),
                       "STORE_32_SP_AT_OFFSET\t$offset, $src", [(store i32: $src, imm: $offset)]>;                    


def LOAD_32_OFFSET_WITH_SP : I8085Pseudo<(outs GR32: $dst),
                       (ins i16imm: $offset),
                       "LOAD_32_OFFSET_WITH_SP\t$dst, $offset", [(set i32: $dst, (load imm: $offset))]>;

def LOAD_32_WITH_IMM_ADDR : I8085Pseudo<(outs GR32: $dst),
                       (ins imm16Addr: $src),
                       "LOAD_32_ADDR $dst, $src", []>;

def LOAD_32_ADDR_CONTENT : I8085Pseudo<(outs GR32: $dst),
                       (ins GR32: $src),
                       "LOAD_32_ADDR_CONTENT $dst, $src", [(set i32: $dst, (load i32: $src))]>;

}

def MVI_32 : I8085Pseudo<(outs GR32: $rd),
                    (ins i32imm: $k),
                    "MVI_32 $rd, $k", [(set i32: $rd, imm: $k)]>;



def ZEXT16TO32
    : I8085Pseudo<(outs GR32: $dst),
                      (ins GR16: $src),
                      "zext16to32 $dst, $src", 
                      [(set i32: $dst, (zext i16: $src))]>;


def SEXT16TO32
    : I8085Pseudo<(outs GR32: $dst),
                      (ins GR16: $src),
                      "sext16to32 $dst, $src", 
                      [(set i32: $dst, (sext i16: $src))]>;

def AEXT16TO32
    : I8085Pseudo<(outs GR32: $dst),
                      (ins GR16: $src),
                      "aext16to32 $dst, $src", 
                      [(set i32: $dst, (anyext i16: $src))]>;                      



def ZEXT8TO32
    : I8085Pseudo<(outs GR32: $dst),
                      (ins GR8: $src),
                      "zext8to32 $dst, $src", 
                      [(set i32: $dst, (zext i8: $src))]>;


def SEXT8TO32
    : I8085Pseudo<(outs GR32: $dst),
                      (ins GR8: $src),
                      "sext8to32 $dst, $src", 
                      [(set i32: $dst, (sext i8: $src))]>;

def AEXT8TO32
    : I8085Pseudo<(outs GR32: $dst),
                      (ins GR8: $src),
                      "aext8to32 $dst, $src", 
                      [(set i32: $dst, (anyext i8: $src))]>; 


def TRUNC32TO16
    : I8085Pseudo<(outs GR16: $dst),
                      (ins GR32: $src),
                      "trunc32to16 $dst, $src",
                      [(set i16
                        : $dst, (trunc i32
                                 : $src))]>; 

def TRUNC32TO8
    : I8085Pseudo<(outs GR8: $dst),
                      (ins GR32: $src),
                      "trunc32to8 $dst, $src",
                      [(set i8
                        : $dst, (trunc i32
                                 : $src))]>;                                 


let Constraints = "$src = $dst", Defs=[SREG]  in {
        def SEXT32_INREG_8
            : I8085Pseudo<(outs GR32: $dst),
                            (ins GR32: $src),
                            "SEXT32_INREG_8 $dst, $src",
                            [(set i32: $dst, (sext_inreg i32: $src,i8))]>;

        def SEXT32_INREG_16
            : I8085Pseudo<(outs GR32: $dst),
                            (ins GR32: $src),
                            "SEXT32_INREG_16 $dst, $src",
                            [(set i32: $dst, (sext_inreg i32: $src,i16))]>;                     

}

def MOV_32 : I8085Pseudo<(outs GR32: $rd),
                    (ins GR32: $rs),
                    "MOV_32 $rd, $rs", []>;


let isBarrier = 1, isBranch = 1, isTerminator = 1 , Defs=[SREG] in {

   def JMP_32_IF_NOT_EQUAL : I8085Pseudo<(outs),(ins GR32: $lhs, GR32: $rhs, addressTarget:$k),
                                           "JMP_32_IF_NOT_EQUAL $lhs, $rhs, $k", []>; 

   def JMP_32_IF_SAME_SIGN : I8085Pseudo<(outs),(ins GR32: $lhs, GR32: $rhs, addressTarget:$k),
                                           "JMP_32_IF_SAME_SIGN $lhs, $rhs, $k", []>;   
   
   def JMP_32_IF_POSITIVE : I8085Pseudo<(outs),(ins GR32: $reg, addressTarget:$k),
                                           "JMP_32_IF_POSITIVE $reg, $k", []>; 
                                                                                 
}

let Constraints = "$lhs = $dst" , Uses=[SREG] in {

def SET_NE_32 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR32: $lhs,GR32: $rhs),
                        "SET_NE_32\t$dst, $lhs, $rhs", []>;

def SET_EQ_32 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR32: $lhs,GR32: $rhs),
                        "SET_EQ_32\t$dst, $lhs, $rhs", []>;

def SET_GT_32 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR32: $lhs,GR32: $rhs),
                        "SET_SGT_32\t$dst, $lhs, $rhs", []>;

def SET_GE_32 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR32: $lhs,GR32: $rhs),
                        "SET_SGE_32\t$dst, $lhs, $rhs", []>;

def SET_LT_32 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR32: $lhs,GR32: $rhs),
                        "SET_SLT_32\t$dst, $lhs, $rhs", []>;

def SET_LE_32 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR32: $lhs,GR32: $rhs),
                        "SET_SLE_32\t$dst, $lhs, $rhs", []>;   

}

let Constraints = "$lhs = $dst" , Uses=[SREG] in {

def SET_UGT_32 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR32: $lhs,GR32: $rhs),
                        "SET_UGT_32\t$dst, $lhs, $rhs", []>;

def SET_UGE_32 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR32: $lhs,GR32: $rhs),
                        "SET_UGE_32\t$dst, $lhs, $rhs", []>;

def SET_ULT_32 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR32: $lhs,GR32: $rhs),
                        "SET_ULT_32\t$dst, $lhs, $rhs", []>;

def SET_ULE_32 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR32: $lhs,GR32: $rhs),
                        "SET_ULE_32\t$dst, $lhs, $rhs", []>;   

}

let Constraints = "$lhs = $dst" , Uses=[SREG] in {

def SET_DIFF_SIGN_GT_32 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR32: $lhs,GR32: $rhs),
                        "SET_DIFF_SIGN_GT_32\t$dst, $lhs, $rhs", []>;

def SET_DIFF_SIGN_LT_32 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR32: $lhs,GR32: $rhs),
                        "SET_DIFF_SIGN_LT_32\t$dst, $lhs, $rhs", []>;                         

}


def imm32_Neg_Form : SDNodeXForm<imm, [{
                    return CurDAG->getTargetConstant(-N->getAPIntValue(),
                                                     SDLoc(N), MVT::i32);
                  }]>;


def IsNeg32 : ImmLeaf<i32, [{
  return Imm < 0;
}], imm32_Neg_Form>;


def : Pat<(add i32: $src1, (IsNeg32 :$src2)),(SUBI_32 i32: $src1, (imm32_Neg_Form imm: $src2))>;


def : Pat<(i32(load(I8085Wrapper tglobaladdr
                    : $dst))),
          (LOAD_32_WITH_IMM_ADDR tglobaladdr
           : $dst)>;
