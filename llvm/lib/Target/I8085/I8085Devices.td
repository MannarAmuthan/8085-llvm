//===---------------------------------------------------------------------===//
// I8085 Device Definitions
//===---------------------------------------------------------------------===//

// :TODO: Implement the skip errata, see `gcc/config/avr/avr-arch.h` for details
// :TODO: We define all devices with SRAM to have all variants of LD/ST/LDD/STD.
//        In reality, avr1 (no SRAM) has one variant each of `LD` and `ST`.
//        avr2 (with SRAM) adds the rest of the variants.

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
def FamilyI80850 : Family<"avr0", []>;

def FamilyI80851 : Family<"avr1", [FamilyI80850, FeatureLPM, FeatureMMR]>;

def FamilyI80852
    : Family<"avr2",
             [FamilyI80851, FeatureIJMPCALL, FeatureADDSUBIW, FeatureSRAM]>;

def FamilyI808525
    : Family<"avr25",
             [FamilyI80852, FeatureMOVW, FeatureLPMX, FeatureSPM, FeatureBREAK]>;

def FamilyI80853 : Family<"avr3", [FamilyI80852, FeatureJMPCALL]>;

def FamilyI808531 : Family<"avr31", [FamilyI80853, FeatureELPM]>;

def FamilyI808535
    : Family<"avr35",
             [FamilyI80853, FeatureMOVW, FeatureLPMX, FeatureSPM, FeatureBREAK]>;

def FamilyI80854 : Family<"avr4",
                        [FamilyI80852, FeatureMultiplication, FeatureMOVW,
                         FeatureLPMX, FeatureSPM, FeatureBREAK]>;

def FamilyI80855 : Family<"avr5",
                        [FamilyI80853, FeatureMultiplication, FeatureMOVW,
                         FeatureLPMX, FeatureSPM, FeatureBREAK]>;

def FamilyI808551 : Family<"avr51", [FamilyI80855, FeatureELPM, FeatureELPMX]>;

def FamilyI80856 : Family<"avr6", [FamilyI808551]>;

def FamilyTiny
    : Family<"avrtiny",
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

//===---------------------------------------------------------------------===//
// I8085 microcontrollers supported.
//===---------------------------------------------------------------------===//

class Device<string Name, Family Fam, ELFArch Arch,
             list<SubtargetFeature> ExtraFeatures = []>
    : Processor<Name, NoItineraries, !listconcat([Fam, Arch], ExtraFeatures)>;

// Generic MCUs
// Note that several versions of GCC has strange ELF architecture
// settings for backwards compatibility - see `gas/config/tc-avr.c`
// in I8085 binutils. We do not replicate this.
def : Device<"avr1", FamilyI80851, ELFArchI80851>;
def : Device<"avr2", FamilyI80852, ELFArchI80852>;
def : Device<"avr25", FamilyI808525, ELFArchI808525>;
def : Device<"avr3", FamilyI80853, ELFArchI80853>;
def : Device<"avr31", FamilyI808531, ELFArchI808531>;
def : Device<"avr35", FamilyI808535, ELFArchI808535>;
def : Device<"avr4", FamilyI80854, ELFArchI80854>;
def : Device<"avr5", FamilyI80855, ELFArchI80855>;
def : Device<"avr51", FamilyI808551, ELFArchI808551>;
def : Device<"avr6", FamilyI80856, ELFArchI80856>;
def : Device<"avrxmega1", FamilyXMEGA, ELFArchXMEGA1>;
def : Device<"avrxmega2", FamilyXMEGA, ELFArchXMEGA2>;
def : Device<"avrxmega3", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"avrxmega4", FamilyXMEGA, ELFArchXMEGA4>;
def : Device<"avrxmega5", FamilyXMEGA, ELFArchXMEGA5>;
def : Device<"avrxmega6", FamilyXMEGA, ELFArchXMEGA6>;
def : Device<"avrxmega7", FamilyXMEGA, ELFArchXMEGA7>;
def : Device<"avrtiny", FamilyTiny, ELFArchTiny>;

// Specific MCUs
// NOTE: This list has been synchronized with gcc-avr 5.4.0 and avr-libc 2.0.0.
def : Device<"at90s1200", FamilyI80850, ELFArchI80851, [FeatureSmallStack]>;
def : Device<"attiny11", FamilyI80851, ELFArchI80851, [FeatureSmallStack]>;
def : Device<"attiny12", FamilyI80851, ELFArchI80851, [FeatureSmallStack]>;
def : Device<"attiny15", FamilyI80851, ELFArchI80851, [FeatureSmallStack]>;
def : Device<"attiny28", FamilyI80851, ELFArchI80851, [FeatureSmallStack]>;
def : Device<"at90s2313", FamilyI80852, ELFArchI80852, [FeatureSmallStack]>;
def : Device<"at90s2323", FamilyI80852, ELFArchI80852, [FeatureSmallStack]>;
def : Device<"at90s2333", FamilyI80852, ELFArchI80852, [FeatureSmallStack]>;
def : Device<"at90s2343", FamilyI80852, ELFArchI80852, [FeatureSmallStack]>;
def : Device<"attiny22", FamilyI80852, ELFArchI80852, [FeatureSmallStack]>;
def : Device<"attiny26", FamilyI80852, ELFArchI80852,
             [FeatureLPMX, FeatureSmallStack]>;
def : Device<"at86rf401", FamilyI80852, ELFArchI808525, [FeatureMOVW, FeatureLPMX]>;
def : Device<"at90s4414", FamilyI80852, ELFArchI80852, [FeatureSmallStack]>;
def : Device<"at90s4433", FamilyI80852, ELFArchI80852, [FeatureSmallStack]>;
def : Device<"at90s4434", FamilyI80852, ELFArchI80852, [FeatureSmallStack]>;
def : Device<"at90s8515", FamilyI80852, ELFArchI80852>;
def : Device<"at90c8534", FamilyI80852, ELFArchI80852>;
def : Device<"at90s8535", FamilyI80852, ELFArchI80852>;
def : Device<"ata5272", FamilyI808525, ELFArchI808525>;
def : Device<"ata6616c", FamilyI808525, ELFArchI808525>;
def : Device<"attiny13", FamilyI808525, ELFArchI808525, [FeatureSmallStack]>;
def : Device<"attiny13a", FamilyI808525, ELFArchI808525, [FeatureSmallStack]>;
def : Device<"attiny2313", FamilyI808525, ELFArchI808525, [FeatureSmallStack]>;
def : Device<"attiny2313a", FamilyI808525, ELFArchI808525, [FeatureSmallStack]>;
def : Device<"attiny24", FamilyI808525, ELFArchI808525, [FeatureSmallStack]>;
def : Device<"attiny24a", FamilyI808525, ELFArchI808525, [FeatureSmallStack]>;
def : Device<"attiny4313", FamilyI808525, ELFArchI808525>;
def : Device<"attiny44", FamilyI808525, ELFArchI808525>;
def : Device<"attiny44a", FamilyI808525, ELFArchI808525>;
def : Device<"attiny84", FamilyI808525, ELFArchI808525>;
def : Device<"attiny84a", FamilyI808525, ELFArchI808525>;
def : Device<"attiny25", FamilyI808525, ELFArchI808525, [FeatureSmallStack]>;
def : Device<"attiny45", FamilyI808525, ELFArchI808525>;
def : Device<"attiny85", FamilyI808525, ELFArchI808525>;
def : Device<"attiny261", FamilyI808525, ELFArchI808525, [FeatureSmallStack]>;
def : Device<"attiny261a", FamilyI808525, ELFArchI808525, [FeatureSmallStack]>;
def : Device<"attiny441", FamilyI808525, ELFArchI808525>;
def : Device<"attiny461", FamilyI808525, ELFArchI808525>;
def : Device<"attiny461a", FamilyI808525, ELFArchI808525>;
def : Device<"attiny841", FamilyI808525, ELFArchI808525>;
def : Device<"attiny861", FamilyI808525, ELFArchI808525>;
def : Device<"attiny861a", FamilyI808525, ELFArchI808525>;
def : Device<"attiny87", FamilyI808525, ELFArchI808525>;
def : Device<"attiny43u", FamilyI808525, ELFArchI808525>;
def : Device<"attiny48", FamilyI808525, ELFArchI808525>;
def : Device<"attiny88", FamilyI808525, ELFArchI808525>;
def : Device<"attiny828", FamilyI808525, ELFArchI808525>;
def : Device<"at43usb355", FamilyI80853, ELFArchI80853>;
def : Device<"at76c711", FamilyI80853, ELFArchI80853>;
def : Device<"atmega103", FamilyI808531, ELFArchI808531>;
def : Device<"at43usb320", FamilyI808531, ELFArchI808531>;
def : Device<"attiny167", FamilyI808535, ELFArchI808535>;
def : Device<"at90usb82", FamilyI808535, ELFArchI808535>;
def : Device<"at90usb162", FamilyI808535, ELFArchI808535>;
def : Device<"ata5505", FamilyI808535, ELFArchI808535>;
def : Device<"ata6617c", FamilyI808535, ELFArchI808535>;
def : Device<"ata664251", FamilyI808535, ELFArchI808535>;
def : Device<"atmega8u2", FamilyI808535, ELFArchI808535>;
def : Device<"atmega16u2", FamilyI808535, ELFArchI808535>;
def : Device<"atmega32u2", FamilyI808535, ELFArchI808535>;
def : Device<"attiny1634", FamilyI808535, ELFArchI808535>;
def : Device<"atmega8", FamilyI80852, ELFArchI80854,
             [FeatureMultiplication, FeatureMOVW, FeatureLPMX, FeatureSPM]>;
def : Device<"ata6289", FamilyI80854, ELFArchI80854>;
def : Device<"atmega8a", FamilyI80852, ELFArchI80854,
             [FeatureMultiplication, FeatureMOVW, FeatureLPMX, FeatureSPM]>;
def : Device<"ata6285", FamilyI80854, ELFArchI80854>;
def : Device<"ata6286", FamilyI80854, ELFArchI80854>;
def : Device<"ata6612c", FamilyI80854, ELFArchI80854>;
def : Device<"atmega48", FamilyI80854, ELFArchI80854>;
def : Device<"atmega48a", FamilyI80854, ELFArchI80854>;
def : Device<"atmega48pa", FamilyI80854, ELFArchI80854>;
def : Device<"atmega48pb", FamilyI80854, ELFArchI80854>;
def : Device<"atmega48p", FamilyI80854, ELFArchI80854>;
def : Device<"atmega88", FamilyI80854, ELFArchI80854>;
def : Device<"atmega88a", FamilyI80854, ELFArchI80854>;
def : Device<"atmega88p", FamilyI80854, ELFArchI80854>;
def : Device<"atmega88pa", FamilyI80854, ELFArchI80854>;
def : Device<"atmega88pb", FamilyI80854, ELFArchI80854>;
def : Device<"atmega8515", FamilyI80852, ELFArchI80854,
             [FeatureMultiplication, FeatureMOVW, FeatureLPMX, FeatureSPM]>;
def : Device<"atmega8535", FamilyI80852, ELFArchI80854,
             [FeatureMultiplication, FeatureMOVW, FeatureLPMX, FeatureSPM]>;
def : Device<"atmega8hva", FamilyI80854, ELFArchI80854>;
def : Device<"at90pwm1", FamilyI80854, ELFArchI80854>;
def : Device<"at90pwm2", FamilyI80854, ELFArchI80854>;
def : Device<"at90pwm2b", FamilyI80854, ELFArchI80854>;
def : Device<"at90pwm3", FamilyI80854, ELFArchI80854>;
def : Device<"at90pwm3b", FamilyI80854, ELFArchI80854>;
def : Device<"at90pwm81", FamilyI80854, ELFArchI80854>;
def : Device<"ata5702m322", FamilyI80855, ELFArchI80855>;
def : Device<"ata5782", FamilyI80855, ELFArchI80855>;
def : Device<"ata5790", FamilyI80855, ELFArchI80855>;
def : Device<"ata5790n", FamilyI80855, ELFArchI80855>;
def : Device<"ata5791", FamilyI80855, ELFArchI80855>;
def : Device<"ata5795", FamilyI80855, ELFArchI80855>;
def : Device<"ata5831", FamilyI80855, ELFArchI80855>;
def : Device<"ata6613c", FamilyI80855, ELFArchI80855>;
def : Device<"ata6614q", FamilyI80855, ELFArchI80855>;
def : Device<"ata8210", FamilyI80855, ELFArchI80855>;
def : Device<"ata8510", FamilyI80855, ELFArchI80855>;
def : Device<"atmega16", FamilyI80855, ELFArchI80855>;
def : Device<"atmega16a", FamilyI80855, ELFArchI80855>;
def : Device<"atmega161", FamilyI80853, ELFArchI80855,
             [FeatureMultiplication, FeatureMOVW, FeatureLPMX, FeatureSPM]>;
def : Device<"atmega162", FamilyI80855, ELFArchI80855>;
def : Device<"atmega163", FamilyI80853, ELFArchI80855,
             [FeatureMultiplication, FeatureMOVW, FeatureLPMX, FeatureSPM]>;
def : Device<"atmega164a", FamilyI80855, ELFArchI80855>;
def : Device<"atmega164p", FamilyI80855, ELFArchI80855>;
def : Device<"atmega164pa", FamilyI80855, ELFArchI80855>;
def : Device<"atmega165", FamilyI80855, ELFArchI80855>;
def : Device<"atmega165a", FamilyI80855, ELFArchI80855>;
def : Device<"atmega165p", FamilyI80855, ELFArchI80855>;
def : Device<"atmega165pa", FamilyI80855, ELFArchI80855>;
def : Device<"atmega168", FamilyI80855, ELFArchI80855>;
def : Device<"atmega168a", FamilyI80855, ELFArchI80855>;
def : Device<"atmega168p", FamilyI80855, ELFArchI80855>;
def : Device<"atmega168pa", FamilyI80855, ELFArchI80855>;
def : Device<"atmega168pb", FamilyI80855, ELFArchI80855>;
def : Device<"atmega169", FamilyI80855, ELFArchI80855>;
def : Device<"atmega169a", FamilyI80855, ELFArchI80855>;
def : Device<"atmega169p", FamilyI80855, ELFArchI80855>;
def : Device<"atmega169pa", FamilyI80855, ELFArchI80855>;
def : Device<"atmega32", FamilyI80855, ELFArchI80855>;
def : Device<"atmega32a", FamilyI80855, ELFArchI80855>;
def : Device<"atmega323", FamilyI80855, ELFArchI80855>;
def : Device<"atmega324a", FamilyI80855, ELFArchI80855>;
def : Device<"atmega324p", FamilyI80855, ELFArchI80855>;
def : Device<"atmega324pa", FamilyI80855, ELFArchI80855>;
def : Device<"atmega324pb", FamilyI80855, ELFArchI80855>;
def : Device<"atmega325", FamilyI80855, ELFArchI80855>;
def : Device<"atmega325a", FamilyI80855, ELFArchI80855>;
def : Device<"atmega325p", FamilyI80855, ELFArchI80855>;
def : Device<"atmega325pa", FamilyI80855, ELFArchI80855>;
def : Device<"atmega3250", FamilyI80855, ELFArchI80855>;
def : Device<"atmega3250a", FamilyI80855, ELFArchI80855>;
def : Device<"atmega3250p", FamilyI80855, ELFArchI80855>;
def : Device<"atmega3250pa", FamilyI80855, ELFArchI80855>;
def : Device<"atmega328", FamilyI80855, ELFArchI80855>;
def : Device<"atmega328p", FamilyI80855, ELFArchI80855>;
def : Device<"atmega328pb", FamilyI80855, ELFArchI80855>;
def : Device<"atmega329", FamilyI80855, ELFArchI80855>;
def : Device<"atmega329a", FamilyI80855, ELFArchI80855>;
def : Device<"atmega329p", FamilyI80855, ELFArchI80855>;
def : Device<"atmega329pa", FamilyI80855, ELFArchI80855>;
def : Device<"atmega3290", FamilyI80855, ELFArchI80855>;
def : Device<"atmega3290a", FamilyI80855, ELFArchI80855>;
def : Device<"atmega3290p", FamilyI80855, ELFArchI80855>;
def : Device<"atmega3290pa", FamilyI80855, ELFArchI80855>;
def : Device<"atmega406", FamilyI80855, ELFArchI80855>;
def : Device<"atmega64", FamilyI80855, ELFArchI80855>;
def : Device<"atmega64a", FamilyI80855, ELFArchI80855>;
def : Device<"atmega640", FamilyI80855, ELFArchI80855>;
def : Device<"atmega644", FamilyI80855, ELFArchI80855>;
def : Device<"atmega644a", FamilyI80855, ELFArchI80855>;
def : Device<"atmega644p", FamilyI80855, ELFArchI80855>;
def : Device<"atmega644pa", FamilyI80855, ELFArchI80855>;
def : Device<"atmega645", FamilyI80855, ELFArchI80855>;
def : Device<"atmega645a", FamilyI80855, ELFArchI80855>;
def : Device<"atmega645p", FamilyI80855, ELFArchI80855>;
def : Device<"atmega649", FamilyI80855, ELFArchI80855>;
def : Device<"atmega649a", FamilyI80855, ELFArchI80855>;
def : Device<"atmega649p", FamilyI80855, ELFArchI80855>;
def : Device<"atmega6450", FamilyI80855, ELFArchI80855>;
def : Device<"atmega6450a", FamilyI80855, ELFArchI80855>;
def : Device<"atmega6450p", FamilyI80855, ELFArchI80855>;
def : Device<"atmega6490", FamilyI80855, ELFArchI80855>;
def : Device<"atmega6490a", FamilyI80855, ELFArchI80855>;
def : Device<"atmega6490p", FamilyI80855, ELFArchI80855>;
def : Device<"atmega64rfr2", FamilyI80855, ELFArchI80855>;
def : Device<"atmega644rfr2", FamilyI80855, ELFArchI80855>;
def : Device<"atmega16hva", FamilyI80855, ELFArchI80855>;
def : Device<"atmega16hva2", FamilyI80855, ELFArchI80855>;
def : Device<"atmega16hvb", FamilyI80855, ELFArchI80855>;
def : Device<"atmega16hvbrevb", FamilyI80855, ELFArchI80855>;
def : Device<"atmega32hvb", FamilyI80855, ELFArchI80855>;
def : Device<"atmega32hvbrevb", FamilyI80855, ELFArchI80855>;
def : Device<"atmega64hve", FamilyI80855, ELFArchI80855>;
def : Device<"atmega64hve2", FamilyI80855, ELFArchI80855>;
def : Device<"at90can32", FamilyI80855, ELFArchI80855>;
def : Device<"at90can64", FamilyI80855, ELFArchI80855>;
def : Device<"at90pwm161", FamilyI80855, ELFArchI80855>;
def : Device<"at90pwm216", FamilyI80855, ELFArchI80855>;
def : Device<"at90pwm316", FamilyI80855, ELFArchI80855>;
def : Device<"atmega32c1", FamilyI80855, ELFArchI80855>;
def : Device<"atmega64c1", FamilyI80855, ELFArchI80855>;
def : Device<"atmega16m1", FamilyI80855, ELFArchI80855>;
def : Device<"atmega32m1", FamilyI80855, ELFArchI80855>;
def : Device<"atmega64m1", FamilyI80855, ELFArchI80855>;
def : Device<"atmega16u4", FamilyI80855, ELFArchI80855>;
def : Device<"atmega32u4", FamilyI80855, ELFArchI80855>;
def : Device<"atmega32u6", FamilyI80855, ELFArchI80855>;
def : Device<"at90usb646", FamilyI80855, ELFArchI80855>;
def : Device<"at90usb647", FamilyI80855, ELFArchI80855>;
def : Device<"at90scr100", FamilyI80855, ELFArchI80855>;
def : Device<"at94k", FamilyI80853, ELFArchI80855,
             [FeatureMultiplication, FeatureMOVW, FeatureLPMX]>;
def : Device<"m3000", FamilyI80855, ELFArchI80855>;
def : Device<"atmega128", FamilyI808551, ELFArchI808551>;
def : Device<"atmega128a", FamilyI808551, ELFArchI808551>;
def : Device<"atmega1280", FamilyI808551, ELFArchI808551>;
def : Device<"atmega1281", FamilyI808551, ELFArchI808551>;
def : Device<"atmega1284", FamilyI808551, ELFArchI808551>;
def : Device<"atmega1284p", FamilyI808551, ELFArchI808551>;
def : Device<"atmega128rfa1", FamilyI808551, ELFArchI808551>;
def : Device<"atmega128rfr2", FamilyI808551, ELFArchI808551>;
def : Device<"atmega1284rfr2", FamilyI808551, ELFArchI808551>;
def : Device<"at90can128", FamilyI808551, ELFArchI808551>;
def : Device<"at90usb1286", FamilyI808551, ELFArchI808551>;
def : Device<"at90usb1287", FamilyI808551, ELFArchI808551>;
def : Device<"atmega2560", FamilyI80856, ELFArchI80856>;
def : Device<"atmega2561", FamilyI80856, ELFArchI80856>;
def : Device<"atmega256rfr2", FamilyI80856, ELFArchI80856>;
def : Device<"atmega2564rfr2", FamilyI80856, ELFArchI80856>;
def : Device<"atxmega16a4", FamilyXMEGA, ELFArchXMEGA2>;
def : Device<"atxmega16a4u", FamilyXMEGAU, ELFArchXMEGA2>;
def : Device<"atxmega16c4", FamilyXMEGAU, ELFArchXMEGA2>;
def : Device<"atxmega16d4", FamilyXMEGA, ELFArchXMEGA2>;
def : Device<"atxmega32a4", FamilyXMEGA, ELFArchXMEGA2>;
def : Device<"atxmega32a4u", FamilyXMEGAU, ELFArchXMEGA2>;
def : Device<"atxmega32c3", FamilyXMEGAU, ELFArchXMEGA2>;
def : Device<"atxmega32c4", FamilyXMEGAU, ELFArchXMEGA2>;
def : Device<"atxmega32d3", FamilyXMEGA, ELFArchXMEGA2>;
def : Device<"atxmega32d4", FamilyXMEGA, ELFArchXMEGA2>;
def : Device<"atxmega32e5", FamilyXMEGAU, ELFArchXMEGA2>;
def : Device<"atxmega16e5", FamilyXMEGAU, ELFArchXMEGA2>;
def : Device<"atxmega8e5", FamilyXMEGAU, ELFArchXMEGA2>;
def : Device<"atxmega64a3", FamilyXMEGA, ELFArchXMEGA4>;
def : Device<"atxmega64a3u", FamilyXMEGAU, ELFArchXMEGA4>;
def : Device<"atxmega64a4u", FamilyXMEGAU, ELFArchXMEGA4>;
def : Device<"atxmega64b1", FamilyXMEGAU, ELFArchXMEGA4>;
def : Device<"atxmega64b3", FamilyXMEGAU, ELFArchXMEGA4>;
def : Device<"atxmega64c3", FamilyXMEGAU, ELFArchXMEGA4>;
def : Device<"atxmega64d3", FamilyXMEGA, ELFArchXMEGA4>;
def : Device<"atxmega64d4", FamilyXMEGA, ELFArchXMEGA4>;
def : Device<"atxmega64a1", FamilyXMEGA, ELFArchXMEGA5>;
def : Device<"atxmega64a1u", FamilyXMEGAU, ELFArchXMEGA5>;
def : Device<"atxmega128a3", FamilyXMEGA, ELFArchXMEGA6>;
def : Device<"atxmega128a3u", FamilyXMEGAU, ELFArchXMEGA6>;
def : Device<"atxmega128b1", FamilyXMEGAU, ELFArchXMEGA6>;
def : Device<"atxmega128b3", FamilyXMEGAU, ELFArchXMEGA6>;
def : Device<"atxmega128c3", FamilyXMEGAU, ELFArchXMEGA6>;
def : Device<"atxmega128d3", FamilyXMEGA, ELFArchXMEGA6>;
def : Device<"atxmega128d4", FamilyXMEGA, ELFArchXMEGA6>;
def : Device<"atxmega192a3", FamilyXMEGA, ELFArchXMEGA6>;
def : Device<"atxmega192a3u", FamilyXMEGAU, ELFArchXMEGA6>;
def : Device<"atxmega192c3", FamilyXMEGAU, ELFArchXMEGA6>;
def : Device<"atxmega192d3", FamilyXMEGA, ELFArchXMEGA6>;
def : Device<"atxmega256a3", FamilyXMEGA, ELFArchXMEGA6>;
def : Device<"atxmega256a3u", FamilyXMEGAU, ELFArchXMEGA6>;
def : Device<"atxmega256a3b", FamilyXMEGA, ELFArchXMEGA6>;
def : Device<"atxmega256a3bu", FamilyXMEGAU, ELFArchXMEGA6>;
def : Device<"atxmega256c3", FamilyXMEGAU, ELFArchXMEGA6>;
def : Device<"atxmega256d3", FamilyXMEGA, ELFArchXMEGA6>;
def : Device<"atxmega384c3", FamilyXMEGAU, ELFArchXMEGA6>;
def : Device<"atxmega384d3", FamilyXMEGA, ELFArchXMEGA6>;
def : Device<"atxmega128a1", FamilyXMEGA, ELFArchXMEGA7>;
def : Device<"atxmega128a1u", FamilyXMEGAU, ELFArchXMEGA7>;
def : Device<"atxmega128a4u", FamilyXMEGAU, ELFArchXMEGA7>;
def : Device<"attiny4", FamilyTiny, ELFArchTiny>;
def : Device<"attiny5", FamilyTiny, ELFArchTiny>;
def : Device<"attiny9", FamilyTiny, ELFArchTiny>;
def : Device<"attiny10", FamilyTiny, ELFArchTiny>;
def : Device<"attiny20", FamilyTiny, ELFArchTiny>;
def : Device<"attiny40", FamilyTiny, ELFArchTiny>;
def : Device<"attiny102", FamilyTiny, ELFArchTiny>;
def : Device<"attiny104", FamilyTiny, ELFArchTiny>;
def : Device<"attiny202", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny402", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny204", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny404", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny804", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny1604", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny406", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny806", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny1606", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny807", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny1607", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny212", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny412", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny214", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny414", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny814", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny1614", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny416", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny816", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny1616", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny3216", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny417", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny817", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny1617", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny3217", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny1624", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny1626", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"attiny1627", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"atmega808", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"atmega809", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"atmega1608", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"atmega1609", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"atmega3208", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"atmega3209", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"atmega4808", FamilyXMEGA3, ELFArchXMEGA3>;
def : Device<"atmega4809", FamilyXMEGA3, ELFArchXMEGA3>;
