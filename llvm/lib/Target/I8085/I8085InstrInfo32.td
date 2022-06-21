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
  def AND_32
      : I8085Pseudo<(outs GR32: $rd),
              (ins GR32: $src, GR32: $rr),
              "AND_32\t$rd, $rr",
              [(set i32: $rd, (and i32: $src, i32: $rr))]>; 

  def ADDI_32
      : I8085Pseudo<(outs GR32: $rd),
              (ins GR32: $src, i32imm: $k),
              "ADDI_32\t$rd, $k",
              [(set i32: $rd, (add i32: $src, imm: $k))]>;                       
}

let Uses=[SP] in {
def LOAD_32 : I8085Pseudo<(outs GR32: $dst),
                       (ins i32imm: $src),
                       "LOAD_32 $dst, $src", [(set i32
                                             : $dst, imm
                                             : $src)]>; 

def STORE_32 : I8085Pseudo<(outs),
                            (ins mspi: $dst, GR32: $src),
                            "STORE_32 $dst, $src", [(store i32
                                                     : $src, addr
                                                     : $dst)]>;

def LOAD_32_WITH_ADDR : I8085Pseudo<(outs GR32: $dst),
                       (ins mspi: $src),
                       "LOAD $dst, $src", [(set i32: $dst, (load addr: $src))]>;    



def STORE_32_AT_OFFSET_WITH_SP : I8085Pseudo<(outs),
                       (ins GR32: $src, i16imm: $offset),
                       "STORE_32_AT_OFFSET_WITH_SP\t$offset, $src", [(store i32: $src, imm: $offset)]>;                    


def LOAD_32_OFFSET_WITH_SP : I8085Pseudo<(outs GR32: $dst),
                       (ins i16imm: $offset),
                       "LOAD_32_OFFSET_WITH_SP\t$dst, $offset", [(set i32: $dst, (load imm: $offset))]>;

}

// def : Pat<(add i32: $src1, (imm :$src2)),(ADD_32 i32: $src1, (LOAD_32 imm: $src2))>;

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


def TRUNC32TO16
    : I8085Pseudo<(outs GR16: $dst),
                      (ins GR32: $src),
                      "trunc32to16 $dst, $src",
                      [(set i16
                        : $dst, (trunc i32
                                 : $src))]>; 


def MOV_32 : I8085Pseudo<(outs GR32: $rd),
                    (ins GR32: $rs),
                    "MOV_32 $rd, $rs", []>;


let isBarrier = 1, isBranch = 1, isTerminator = 1 , Defs=[SREG] in {
//    def JMP_8_IF : I8085Pseudo<(outs),(ins GR8: $reg, addressTarget:$k),
//                                            "JMP_8_IF $reg, $k", []>;

//    def JMP_16_IF : I8085Pseudo<(outs),(ins GR16: $reg, addressTarget:$k),
//                                            "JMP_16_IF $reg, $k", []>; 

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