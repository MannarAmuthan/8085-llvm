//===---------------------------------------------------------------------===//
// I8085 Device Definitions
//===---------------------------------------------------------------------===//

// :TODO: Implement the skip errata, see `gcc/config/i8085/i8085-arch.h` for details
// :TODO: We define all devices with SRAM to have all variants of LD/ST/LDD/STD.
//        In reality, i80851 (no SRAM) has one variant each of `LD` and `ST`.
//        i80852 (with SRAM) adds the rest of the variants.

// A feature set aggregates features, grouping them. We don't want to create a
// new member in I8085Subtarget (to store a value) for each set because we do not
// care if the set is supported, only the subfeatures inside the set. We fix
// this by simply setting the same dummy member for all feature sets, which is
// then ignored.
class FeatureSet<string name, string desc, list<SubtargetFeature> i>
    : SubtargetFeature<name, "m_FeatureSetDummy", "true", desc, i>;

// A family of microcontrollers, defining a set of supported features.
class Family<string name, list<SubtargetFeature> i>
    : FeatureSet<
          name, !strconcat("The device is a part of the ", name, " family"), i>;

// The device has SRAM, and supports the bare minimum of
// SRAM-relevant instructions.
//
// These are:
// LD - all 9 variants
// ST - all 9 variants
// LDD - two variants for Y and Z
// STD - two variants for Y and Z
// `LDS Rd, K`
// `STS k, Rr`
// `PUSH`/`POP`
def FeatureSRAM : SubtargetFeature<"sram", "m_hasSRAM", "true",
                                   "The device has random access memory">;

// The device supports the `JMP k` and `CALL k` instructions.
def FeatureJMPCALL : SubtargetFeature<"jmpcall", "m_hasJMPCALL", "true",
                                      "The device supports the `JMP` and "
                                      "`CALL` instructions">;

// The device supports the indirect branches `IJMP` and `ICALL`.
def FeatureIJMPCALL : SubtargetFeature<"ijmpcall", "m_hasIJMPCALL", "true",
                                       "The device supports `IJMP`/`ICALL`"
                                       "instructions">;

// The device supports the extended indirect branches `EIJMP` and `EICALL`.
def FeatureEIJMPCALL : SubtargetFeature<"eijmpcall", "m_hasEIJMPCALL", "true",
                                        "The device supports the "
                                        "`EIJMP`/`EICALL` instructions">;

// The device supports `ADDI Rd, K`, `SUBI Rd, K`.
def FeatureADDSUBIW : SubtargetFeature<"addsubiw", "m_hasADDSUBIW", "true",
                                       "Enable 16-bit register-immediate "
                                       "addition and subtraction instructions">;

// The device has an 8-bit stack pointer (SP) register.
def FeatureSmallStack
    : SubtargetFeature<"smallstack", "m_hasSmallStack", "true",
                       "The device has an 8-bit "
                       "stack pointer">;

// The device supports the 16-bit GPR pair MOVW instruction.
def FeatureMOVW : SubtargetFeature<"movw", "m_hasMOVW", "true",
                                   "The device supports the 16-bit MOVW "
                                   "instruction">;

// The device supports the `LPM` instruction, with implied destination being r0.
def FeatureLPM : SubtargetFeature<"lpm", "m_hasLPM", "true",
                                  "The device supports the `LPM` instruction">;

// The device supports the `LPM Rd, Z[+] instruction.
def FeatureLPMX : SubtargetFeature<"lpmx", "m_hasLPMX", "true",
                                   "The device supports the `LPM Rd, Z[+]` "
                                   "instruction">;

// The device supports the `ELPM` instruction.
def FeatureELPM : SubtargetFeature<"elpm", "m_hasELPM", "true",
                                   "The device supports the ELPM instruction">;

// The device supports the `ELPM Rd, Z[+]` instructions.
def FeatureELPMX : SubtargetFeature<"elpmx", "m_hasELPMX", "true",
                                    "The device supports the `ELPM Rd, Z[+]` "
                                    "instructions">;

// The device supports the `SPM` instruction.
def FeatureSPM : SubtargetFeature<"spm", "m_hasSPM", "true",
                                  "The device supports the `SPM` instruction">;

// The device supports the `SPM Z+` instruction.
def FeatureSPMX : SubtargetFeature<"spmx", "m_hasSPMX", "true",
                                   "The device supports the `SPM Z+` "
                                   "instruction">;

// The device supports the `DES k` instruction.
def FeatureDES : SubtargetFeature<"des", "m_hasDES", "true",
                                  "The device supports the `DES k` encryption "
                                  "instruction">;

// The device supports the Read-Write-Modify instructions
// XCH, LAS, LAC, and LAT.
def FeatureRMW : SubtargetFeature<"rmw", "m_supportsRMW", "true",
                                  "The device supports the read-write-modify "
                                  "instructions: XCH, LAS, LAC, LAT">;

// The device supports the `[F]MUL[S][U]` family of instructions.
def FeatureMultiplication
    : SubtargetFeature<"mul", "m_supportsMultiplication", "true",
                       "The device supports the "
                       "multiplication instructions">;

// The device supports the `BREAK` instruction.
def FeatureBREAK : SubtargetFeature<"break", "m_hasBREAK", "true",
                                    "The device supports the `BREAK` debugging "
                                    "instruction">;

// The device has instruction encodings specific to the Tiny core.
def FeatureTinyEncoding
    : SubtargetFeature<"tinyencoding", "m_hasTinyEncoding", "true",
                       "The device has Tiny core specific "
                       "instruction encodings">;

// The device has CPU registers mapped in data address space
def FeatureMMR : SubtargetFeature<"memmappedregs", "m_hasMemMappedGPR", "true",
                                  "The device has CPU registers "
                                  "mapped in data address space">;

class ELFArch<string name>
    : SubtargetFeature<"", "ELFArch", !strconcat("ELF::", name), "">;

// ELF e_flags architecture values
def ELFArchI80851 : ELFArch<"EF_I8085_ARCH_I80851">;
def ELFArchI80852 : ELFArch<"EF_I8085_ARCH_I80852">;
def ELFArchI808525 : ELFArch<"EF_I8085_ARCH_I808525">;
def ELFArchI80853 : ELFArch<"EF_I8085_ARCH_I80853">;
def ELFArchI808531 : ELFArch<"EF_I8085_ARCH_I808531">;
def ELFArchI808535 : ELFArch<"EF_I8085_ARCH_I808535">;
def ELFArchI80854 : ELFArch<"EF_I8085_ARCH_I80854">;
def ELFArchI80855 : ELFArch<"EF_I8085_ARCH_I80855">;
def ELFArchI808551 : ELFArch<"EF_I8085_ARCH_I808551">;
def ELFArchI80856 : ELFArch<"EF_I8085_ARCH_I80856">;
def ELFArchTiny : ELFArch<"EF_I8085_ARCH_I8085TINY">;
def ELFArchXMEGA1 : ELFArch<"EF_I8085_ARCH_XMEGA1">;
def ELFArchXMEGA2 : ELFArch<"EF_I8085_ARCH_XMEGA2">;
def ELFArchXMEGA3 : ELFArch<"EF_I8085_ARCH_XMEGA3">;
def ELFArchXMEGA4 : ELFArch<"EF_I8085_ARCH_XMEGA4">;
def ELFArchXMEGA5 : ELFArch<"EF_I8085_ARCH_XMEGA5">;
def ELFArchXMEGA6 : ELFArch<"EF_I8085_ARCH_XMEGA6">;
def ELFArchXMEGA7 : ELFArch<"EF_I8085_ARCH_XMEGA7">;

//===---------------------------------------------------------------------===//
// I8085 Families
//===---------------------------------------------------------------------===//

// The device has at least the bare minimum that **every** single I8085
// device should have.
def FamilyI80850 : Family<"i80850", []>;

def FamilyI80851 : Family<"i80851", [FamilyI80850, FeatureLPM, FeatureMMR]>;

def FamilyI80852
    : Family<"i80852",
             [FamilyI80851, FeatureIJMPCALL, FeatureADDSUBIW, FeatureSRAM]>;

def FamilyI808525
    : Family<"i808525",
             [FamilyI80852, FeatureMOVW, FeatureLPMX, FeatureSPM, FeatureBREAK]>;

def FamilyI80853 : Family<"i80853", [FamilyI80852, FeatureJMPCALL]>;

def FamilyI808531 : Family<"i808531", [FamilyI80853, FeatureELPM]>;

def FamilyI808535
    : Family<"i808535",
             [FamilyI80853, FeatureMOVW, FeatureLPMX, FeatureSPM, FeatureBREAK]>;

def FamilyI80854 : Family<"i80854",
                        [FamilyI80852, FeatureMultiplication, FeatureMOVW,
                         FeatureLPMX, FeatureSPM, FeatureBREAK]>;

def FamilyI80855 : Family<"i80855",
                        [FamilyI80853, FeatureMultiplication, FeatureMOVW,
                         FeatureLPMX, FeatureSPM, FeatureBREAK]>;

def FamilyI808551 : Family<"i808551", [FamilyI80855, FeatureELPM, FeatureELPMX]>;

def FamilyI80856 : Family<"i80856", [FamilyI808551]>;

def FamilyTiny
    : Family<"i8085tiny",
             [FamilyI80850, FeatureBREAK, FeatureSRAM, FeatureTinyEncoding,
              FeatureSmallStack]>;

def FamilyXMEGA3 : Family<"xmega3",
                          [FamilyI80850, FeatureLPM, FeatureIJMPCALL,
                           FeatureADDSUBIW, FeatureSRAM, FeatureJMPCALL,
                           FeatureMultiplication, FeatureMOVW, FeatureLPMX,
                           FeatureBREAK]>;

def FamilyXMEGA : Family<"xmega",
                         [FamilyI80850, FeatureLPM, FeatureIJMPCALL,
                          FeatureADDSUBIW, FeatureSRAM, FeatureJMPCALL,
                          FeatureMultiplication, FeatureMOVW, FeatureLPMX,
                          FeatureSPM, FeatureBREAK, FeatureEIJMPCALL,
                          FeatureSPMX, FeatureDES, FeatureELPM, FeatureELPMX]>;

def FamilyXMEGAU : Family<"xmegau", [FamilyXMEGA, FeatureRMW]>;

def FeatureSetSpecial
    : FeatureSet<"special",
                 "Enable use of the entire instruction "
                 "set - used for debugging",
                 [
                   FeatureSRAM, FeatureJMPCALL, FeatureIJMPCALL,
                   FeatureEIJMPCALL, FeatureADDSUBIW, FeatureMOVW, FeatureLPM,
                   FeatureLPMX, FeatureELPM, FeatureELPMX, FeatureSPM,
                   FeatureSPMX, FeatureDES, FeatureRMW, FeatureMultiplication,
                   FeatureBREAK, FeatureMMR
                 ]>;

