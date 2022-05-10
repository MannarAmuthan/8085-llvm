
let Constraints = "$lhs = $dst" , Uses=[SREG] in {
def SET_NE_8 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_NE_8\t$dst, $lhs, $rhs", []>;

def SET_EQ_8 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_EQ_8\t$dst, $lhs, $rhs", []>;

def SET_GT_8 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_SGT_8\t$dst, $lhs, $rhs", []>;

def SET_GE_8 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_SGE_8\t$dst, $lhs, $rhs", []>;

def SET_LT_8 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_SLT_8\t$dst, $lhs, $rhs", []>;

def SET_LE_8 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_SLE_8\t$dst, $lhs, $rhs", []>;   

}

let Constraints = "$lhs = $dst" , Uses=[SREG] in {

def SET_UGT_8 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_UGT_8\t$dst, $lhs, $rhs", []>;

def SET_UGE_8 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_UGE_8\t$dst, $lhs, $rhs", []>;

def SET_ULT_8 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_ULT_8\t$dst, $lhs, $rhs", []>;

def SET_ULE_8 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_ULE_8\t$dst, $lhs, $rhs", []>;   

}

let Constraints = "$lhs = $dst" , Uses=[SREG] in {

def SET_DIFF_SIGN_GT_8 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_DIFF_SIGN_GT_8\t$dst, $lhs, $rhs", []>;

def SET_DIFF_SIGN_GE_8 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_DIFF_SIGN_GE_8\t$dst, $lhs, $rhs", []>;

def SET_DIFF_SIGN_LT_8 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_DIFF_SIGN_LT_8\t$dst, $lhs, $rhs", []>;

def SET_DIFF_SIGN_LE_8 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_DIFF_SIGN_LE_8\t$dst, $lhs, $rhs", []>;                         

}



let Constraints = "$lhs = $dst" , Uses=[SREG] in {

def SET_UGT_16 : I8085InserterPseudo<(outs GR16: $dst),
                        (ins GR16: $lhs,GR16: $rhs),
                        "SET_UGT_16\t$dst, $lhs, $rhs", []>;

def SET_UGE_16 : I8085InserterPseudo<(outs GR16: $dst),
                        (ins GR16: $lhs,GR16: $rhs),
                        "SET_UGE_16\t$dst, $lhs, $rhs", []>;

def SET_ULT_16 : I8085InserterPseudo<(outs GR16: $dst),
                        (ins GR16: $lhs,GR16: $rhs),
                        "SET_ULT_16\t$dst, $lhs, $rhs", []>;

def SET_ULE_16 : I8085InserterPseudo<(outs GR16: $dst),
                        (ins GR16: $lhs,GR16: $rhs),
                        "SET_ULE_16\t$dst, $lhs, $rhs", []>;   

}

let Constraints = "$lhs = $dst" , Uses=[SREG] in {

def SET_DIFF_SIGN_GT_16 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR16: $lhs,GR16: $rhs),
                        "SET_DIFF_SIGN_GT_16\t$dst, $lhs, $rhs", []>;

def SET_DIFF_SIGN_LT_16 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR16: $lhs,GR16: $rhs),
                        "SET_DIFF_SIGN_LT_16\t$dst, $lhs, $rhs", []>;                         

}

let Constraints = "$lhs = $dst" , Uses=[SREG] in {
def SET_NE_16 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR16: $lhs,GR16: $rhs),
                        "SET_NE_16\t$dst, $lhs, $rhs", []>;

def SET_EQ_16 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR16: $lhs,GR16: $rhs),
                        "SET_EQ_16\t$dst, $lhs, $rhs", []>;

def SET_GT_16 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR16: $lhs,GR16: $rhs),
                        "SET_SGT_16\t$dst, $lhs, $rhs", []>;

def SET_GE_16 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR16: $lhs,GR16: $rhs),
                        "SET_SGE_16\t$dst, $lhs, $rhs", []>;

def SET_LT_16 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR16: $lhs,GR16: $rhs),
                        "SET_SLT_16\t$dst, $lhs, $rhs", []>;

def SET_LE_16 : I8085InserterPseudo<(outs GR8: $dst),
                        (ins GR16: $lhs,GR16: $rhs),
                        "SET_SLE_16\t$dst, $lhs, $rhs", []>;   
}

let isBarrier = 1, isBranch = 1, Uses=[SREG]  in {                                                                                                                                   

   def JMP_8_IF : I8085Pseudo<(outs),(ins GR8: $reg, addressTarget:$k),
                                           "JMP_8_IF $reg, $k", []>;

   def JMP_16_IF : I8085Pseudo<(outs),(ins GR16: $reg, addressTarget:$k),
                                           "JMP_16_IF $reg, $k", []>;                                         

   def JZ :  I8085Pseudo<(outs),(ins addressTarget: $label),"JZ\t$label", []>;

   def JNZ :  I8085Pseudo<(outs),(ins addressTarget: $label),"JNZ\t$label", []>;

   def JNC :  I8085Pseudo<(outs),(ins addressTarget: $label),"JNC\t$label", []>;

   def JC :  I8085Pseudo<(outs),(ins addressTarget: $label),"JC\t$label", []>;

   def JP :  I8085Pseudo<(outs),(ins addressTarget: $label),"JP\t$label", []>;

   def JM :  I8085Pseudo<(outs),(ins addressTarget: $label),"JM\t$label", []>;
}

let isBarrier = 1, isBranch = 1, isTerminator = 1 in {
   def JMP : I8085Pseudo<(outs),(ins addressTarget:$k),
                                           "JMP $k", [(br bb: $k)]>; 
}


let isBarrier = 1, isBranch = 1, isTerminator = 1 , Defs=[SREG] in {
   def JMP_16_IF_NOT_EQUAL : I8085Pseudo<(outs),(ins GR16: $lhs, GR16: $rhs, addressTarget:$k),
                                           "JMP_16_IF_NOT_EQUAL $lhs, $rhs, $k", []>; 

   def JMP_16_IF_SAME_SIGN : I8085Pseudo<(outs),(ins GR16: $lhs, GR16: $rhs, addressTarget:$k),
                                           "JMP_16_IF_SAME_SIGN $lhs, $rhs, $k", []>;   
   
   def JMP_16_IF_POSITIVE : I8085Pseudo<(outs),(ins GR16: $reg, addressTarget:$k),
                                           "JMP_16_IF_POSITIVE $reg, $k", []>;   
                                                                                 
}

let Constraints = "$src = $dst" in {
def INV : I8085Pseudo<(outs GR8: $dst),
                        (ins GR8: $src),
                        "INVERSE\t$src", []>; 
}

let Defs=[SREG] in {
def CMP : I8085Pseudo<(outs),(ins GR8: $reg),"CMP\t$reg", []>;
}

def CMA : I8085Pseudo<(outs),(ins),"CMA", []>;

