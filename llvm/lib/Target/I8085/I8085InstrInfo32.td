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
                       "STORE_32_SP_AT_OFFSET\t$offset, $src", [(store i32: $src, imm: $offset)]>;                    

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
