
let Constraints = "$lhs = $dst" in {
def SET_NE_8 : I8085Pseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_NE_8\t$dst, $lhs, $rhs", []>;

def SET_EQ_8 : I8085Pseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_EQ_8\t$dst, $lhs, $rhs", []>;

def SET_GT_8 : I8085Pseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_SGT_8\t$dst, $lhs, $rhs", []>;

def SET_GE_8 : I8085Pseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_SGE_8\t$dst, $lhs, $rhs", []>;

def SET_LT_8 : I8085Pseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_SLT_8\t$dst, $lhs, $rhs", []>;

def SET_LE_8 : I8085Pseudo<(outs GR8: $dst),
                        (ins GR8: $lhs,GR8: $rhs),
                        "SET_SLE_8\t$dst, $lhs, $rhs", []>;   

}

let Constraints = "$lhs = $dst" in {
def SET_NE_16 : I8085Pseudo<(outs GR16: $dst),
                        (ins GR16: $lhs,GR16: $rhs),
                        "SET_NE_16\t$dst, $lhs, $rhs", []>;

def SET_EQ_16 : I8085Pseudo<(outs GR16: $dst),
                        (ins GR16: $lhs,GR16: $rhs),
                        "SET_EQ_16\t$dst, $lhs, $rhs", []>;

def SET_GT_16 : I8085Pseudo<(outs GR16: $dst),
                        (ins GR16: $lhs,GR16: $rhs),
                        "SET_SGT_16\t$dst, $lhs, $rhs", []>;

def SET_GE_16 : I8085Pseudo<(outs GR16: $dst),
                        (ins GR16: $lhs,GR16: $rhs),
                        "SET_SGE_16\t$dst, $lhs, $rhs", []>;

def SET_LT_16 : I8085Pseudo<(outs GR16: $dst),
                        (ins GR16: $lhs,GR16: $rhs),
                        "SET_SLT_16\t$dst, $lhs, $rhs", []>;

def SET_LE_16 : I8085Pseudo<(outs GR16: $dst),
                        (ins GR16: $lhs,GR16: $rhs),
                        "SET_SLE_16\t$dst, $lhs, $rhs", []>;   

}

let isBarrier = 1, isBranch = 1, isTerminator = 1 in {                                                                                                                                     
   def JMP : I8085Pseudo<(outs),(ins brtarget_13:$k),
                                           "JMP $k", [(br bb: $k)]>; 
}

def JMP_8_IF : I8085Pseudo<(outs),(ins GR8: $reg, brtarget_13:$k),
                                           "JMP_8_IF $reg, $k", []>;

def JMP_16_IF : I8085Pseudo<(outs),(ins GR16: $reg, brtarget_13:$k),
                                           "JMP_16_IF $reg, $k", []>;   


let Constraints = "$src = $dst" in {
def INV : I8085Pseudo<(outs GR8: $dst),
                        (ins GR8: $src),
                        "INVERSE\t$src", []>; 
}

def CMP : I8085Pseudo<(outs),(ins GR8: $reg),"CMP\t$reg", []>;

def CMA : I8085Pseudo<(outs),(ins),"CMA", []>;

def JZ :  I8085Pseudo<(outs),(ins brtarget_13: $label),"JZ\t$label", []>;

def JNZ :  I8085Pseudo<(outs),(ins brtarget_13: $label),"JNZ\t$label", []>;