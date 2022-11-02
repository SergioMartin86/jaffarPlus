auto SM83::disassemble(uint16 pc) -> string {
  return {
    hex(pc, 4L), "  ",
    pad(disassembleOpcode(pc), -16, ' '), "  ",
    " AF:", hex(AF, 4L),
    " BC:", hex(BC, 4L),
    " DE:", hex(DE, 4L),
    " HL:", hex(HL, 4L),
    " SP:", hex(SP, 4L)
  };
}

auto SM83::disassembleOpcode(uint16 pc) -> string {
  auto opcode = readDebugger(pc);
  auto lo = readDebugger(pc + 1);
  auto hi = readDebugger(pc + 2);
  auto word = hi << 8 | lo << 0;

  switch(opcode) {
  case 0x00: return {"nop"};
  case 0x01: return {"ld   bc,$", hex(word, 4L)};
  case 0x02: return {"ld   (bc),a"};
  case 0x03: return {"inc  bc"};
  case 0x04: return {"inc  b"};
  case 0x05: return {"dec  b"};
  case 0x06: return {"ld   b,$", hex(lo, 2L)};
  case 0x07: return {"rlca"};
  case 0x08: return {"ld   ($", hex(word, 4L), "),sp"};
  case 0x09: return {"add  hl,bc"};
  case 0x0a: return {"ld   a,(bc)"};
  case 0x0b: return {"dec  bc"};
  case 0x0c: return {"inc  c"};
  case 0x0d: return {"dec  c"};
  case 0x0e: return {"ld   c,$", hex(lo, 2L)};
  case 0x0f: return {"rrca"};
  case 0x10: return {"stop"};
  case 0x11: return {"ld   de,$", hex(word, 4L)};
  case 0x12: return {"ld   (de),a"};
  case 0x13: return {"inc  de"};
  case 0x14: return {"inc  d"};
  case 0x15: return {"dec  d"};
  case 0x16: return {"ld   d,$", hex(lo, 2L)};
  case 0x17: return {"rla"};
  case 0x18: return {"jr   $", hex(pc + 2 + (int8)lo, 4L)};
  case 0x19: return {"add  hl,de"};
  case 0x1a: return {"ld   a,(de)"};
  case 0x1b: return {"dec  de"};
  case 0x1c: return {"inc  e"};
  case 0x1d: return {"dec  e"};
  case 0x1e: return {"ld   e,$", hex(lo, 2L)};
  case 0x1f: return {"rra"};
  case 0x20: return {"jr   nz,$", hex(pc + 2 + (int8)lo, 4L)};
  case 0x21: return {"ld   hl,$", hex(word, 4L)};
  case 0x22: return {"ldi  (hl),a"};
  case 0x23: return {"inc  hl"};
  case 0x24: return {"inc  h"};
  case 0x25: return {"dec  h"};
  case 0x26: return {"ld   h,$", hex(lo, 2L)};
  case 0x27: return {"daa"};
  case 0x28: return {"jr   z,$", hex(pc + 2 + (int8)lo, 4L)};
  case 0x29: return {"add  hl,hl"};
  case 0x2a: return {"ldi  a,(hl)"};
  case 0x2b: return {"dec  hl"};
  case 0x2c: return {"inc  l"};
  case 0x2d: return {"dec  l"};
  case 0x2e: return {"ld   l,$", hex(lo, 2L)};
  case 0x2f: return {"cpl"};
  case 0x30: return {"jr   nc,$", hex(pc + 2 + (int8)lo, 4L)};
  case 0x31: return {"ld   sp,$", hex(word, 4L)};
  case 0x32: return {"ldd  (hl),a"};
  case 0x33: return {"inc  sp"};
  case 0x34: return {"inc  (hl)"};
  case 0x35: return {"dec  (hl)"};
  case 0x36: return {"ld   (hl),$", hex(lo, 2L)};
  case 0x37: return {"scf"};
  case 0x38: return {"jr   c,$", hex(pc + 2 + (int8)lo, 4L)};
  case 0x39: return {"add  hl,sp"};
  case 0x3a: return {"ldd  a,(hl)"};
  case 0x3b: return {"dec  sp"};
  case 0x3c: return {"inc  a"};
  case 0x3d: return {"dec  a"};
  case 0x3e: return {"ld   a,$", hex(lo, 2L)};
  case 0x3f: return {"ccf"};
  case 0x40: return {"ld   b,b"};
  case 0x41: return {"ld   b,c"};
  case 0x42: return {"ld   b,d"};
  case 0x43: return {"ld   b,e"};
  case 0x44: return {"ld   b,h"};
  case 0x45: return {"ld   b,l"};
  case 0x46: return {"ld   b,(hl)"};
  case 0x47: return {"ld   b,a"};
  case 0x48: return {"ld   c,b"};
  case 0x49: return {"ld   c,c"};
  case 0x4a: return {"ld   c,d"};
  case 0x4b: return {"ld   c,e"};
  case 0x4c: return {"ld   c,h"};
  case 0x4d: return {"ld   c,l"};
  case 0x4e: return {"ld   c,(hl)"};
  case 0x4f: return {"ld   c,a"};
  case 0x50: return {"ld   d,b"};
  case 0x51: return {"ld   d,c"};
  case 0x52: return {"ld   d,d"};
  case 0x53: return {"ld   d,e"};
  case 0x54: return {"ld   d,h"};
  case 0x55: return {"ld   d,l"};
  case 0x56: return {"ld   d,(hl)"};
  case 0x57: return {"ld   d,a"};
  case 0x58: return {"ld   e,b"};
  case 0x59: return {"ld   e,c"};
  case 0x5a: return {"ld   e,d"};
  case 0x5b: return {"ld   e,e"};
  case 0x5c: return {"ld   e,h"};
  case 0x5d: return {"ld   e,l"};
  case 0x5e: return {"ld   e,(hl)"};
  case 0x5f: return {"ld   e,a"};
  case 0x60: return {"ld   h,b"};
  case 0x61: return {"ld   h,c"};
  case 0x62: return {"ld   h,d"};
  case 0x63: return {"ld   h,e"};
  case 0x64: return {"ld   h,h"};
  case 0x65: return {"ld   h,l"};
  case 0x66: return {"ld   h,(hl)"};
  case 0x67: return {"ld   h,a"};
  case 0x68: return {"ld   l,b"};
  case 0x69: return {"ld   l,c"};
  case 0x6a: return {"ld   l,d"};
  case 0x6b: return {"ld   l,e"};
  case 0x6c: return {"ld   l,h"};
  case 0x6d: return {"ld   l,l"};
  case 0x6e: return {"ld   l,(hl)"};
  case 0x6f: return {"ld   l,a"};
  case 0x70: return {"ld   (hl),b"};
  case 0x71: return {"ld   (hl),c"};
  case 0x72: return {"ld   (hl),d"};
  case 0x73: return {"ld   (hl),e"};
  case 0x74: return {"ld   (hl),h"};
  case 0x75: return {"ld   (hl),l"};
  case 0x76: return {"halt"};
  case 0x77: return {"ld   (hl),a"};
  case 0x78: return {"ld   a,b"};
  case 0x79: return {"ld   a,c"};
  case 0x7a: return {"ld   a,d"};
  case 0x7b: return {"ld   a,e"};
  case 0x7c: return {"ld   a,h"};
  case 0x7d: return {"ld   a,l"};
  case 0x7e: return {"ld   a,(hl)"};
  case 0x7f: return {"ld   a,a"};
  case 0x80: return {"add  a,b"};
  case 0x81: return {"add  a,c"};
  case 0x82: return {"add  a,d"};
  case 0x83: return {"add  a,e"};
  case 0x84: return {"add  a,h"};
  case 0x85: return {"add  a,l"};
  case 0x86: return {"add  a,(hl)"};
  case 0x87: return {"add  a,a"};
  case 0x88: return {"adc  a,b"};
  case 0x89: return {"adc  a,c"};
  case 0x8a: return {"adc  a,d"};
  case 0x8b: return {"adc  a,e"};
  case 0x8c: return {"adc  a,h"};
  case 0x8d: return {"adc  a,l"};
  case 0x8e: return {"adc  a,(hl)"};
  case 0x8f: return {"adc  a,a"};
  case 0x90: return {"sub  a,b"};
  case 0x91: return {"sub  a,c"};
  case 0x92: return {"sub  a,d"};
  case 0x93: return {"sub  a,e"};
  case 0x94: return {"sub  a,h"};
  case 0x95: return {"sub  a,l"};
  case 0x96: return {"sub  a,(hl)"};
  case 0x97: return {"sub  a,a"};
  case 0x98: return {"sbc  a,b"};
  case 0x99: return {"sbc  a,c"};
  case 0x9a: return {"sbc  a,d"};
  case 0x9b: return {"sbc  a,e"};
  case 0x9c: return {"sbc  a,h"};
  case 0x9d: return {"sbc  a,l"};
  case 0x9e: return {"sbc  a,(hl)"};
  case 0x9f: return {"sbc  a,a"};
  case 0xa0: return {"and  a,b"};
  case 0xa1: return {"and  a,c"};
  case 0xa2: return {"and  a,d"};
  case 0xa3: return {"and  a,e"};
  case 0xa4: return {"and  a,h"};
  case 0xa5: return {"and  a,l"};
  case 0xa6: return {"and  a,(hl)"};
  case 0xa7: return {"and  a,a"};
  case 0xa8: return {"xor  a,b"};
  case 0xa9: return {"xor  a,c"};
  case 0xaa: return {"xor  a,d"};
  case 0xab: return {"xor  a,e"};
  case 0xac: return {"xor  a,h"};
  case 0xad: return {"xor  a,l"};
  case 0xae: return {"xor  a,(hl)"};
  case 0xaf: return {"xor  a,a"};
  case 0xb0: return {"or   a,b"};
  case 0xb1: return {"or   a,c"};
  case 0xb2: return {"or   a,d"};
  case 0xb3: return {"or   a,e"};
  case 0xb4: return {"or   a,h"};
  case 0xb5: return {"or   a,l"};
  case 0xb6: return {"or   a,(hl)"};
  case 0xb7: return {"or   a,a"};
  case 0xb8: return {"cp   a,b"};
  case 0xb9: return {"cp   a,c"};
  case 0xba: return {"cp   a,d"};
  case 0xbb: return {"cp   a,e"};
  case 0xbc: return {"cp   a,h"};
  case 0xbd: return {"cp   a,l"};
  case 0xbe: return {"cp   a,(hl)"};
  case 0xbf: return {"cp   a,a"};
  case 0xc0: return {"ret  nz"};
  case 0xc1: return {"pop  bc"};
  case 0xc2: return {"jp   nz,$", hex(word, 4L)};
  case 0xc3: return {"jp   $", hex(word, 4L)};
  case 0xc4: return {"call nz,$", hex(word, 4L)};
  case 0xc5: return {"push bc"};
  case 0xc6: return {"add  a,$", hex(lo, 2L)};
  case 0xc7: return {"rst  $0000"};
  case 0xc8: return {"ret  z"};
  case 0xc9: return {"ret"};
  case 0xca: return {"jp   z,$", hex(word, 4L)};
  case 0xcb: return disassembleOpcodeCB(pc + 1);
  case 0xcc: return {"call z,$", hex(word, 4L)};
  case 0xcd: return {"call $", hex(word, 4L)};
  case 0xce: return {"adc  a,$", hex(lo, 2L)};
  case 0xcf: return {"rst  $0008"};
  case 0xd0: return {"ret  nc"};
  case 0xd1: return {"pop  de"};
  case 0xd2: return {"jp   nc,$", hex(word, 4L)};
  case 0xd4: return {"call nc,$", hex(word, 4L)};
  case 0xd5: return {"push de"};
  case 0xd6: return {"sub  a,$", hex(lo, 2L)};
  case 0xd7: return {"rst  $0010"};
  case 0xd8: return {"ret  c"};
  case 0xd9: return {"reti"};
  case 0xda: return {"jp   c,$", hex(word, 4L)};
  case 0xdc: return {"call c,$", hex(word, 4L)};
  case 0xde: return {"sbc  a,$", hex(lo, 2L)};
  case 0xdf: return {"rst  $0018"};
  case 0xe0: return {"ldh  ($ff", hex(lo, 2L), "),a"};
  case 0xe1: return {"pop  hl"};
  case 0xe2: return {"ldh  ($ff00+c),a"};
  case 0xe5: return {"push hl"};
  case 0xe6: return {"and  a,$", hex(lo, 2L)};
  case 0xe7: return {"rst  $0020"};
  case 0xe8: return {"add  sp,$", hex((int8)lo, 4L)};
  case 0xe9: return {"jp   hl"};
  case 0xea: return {"ld   ($", hex(word, 4L), "),a"};
  case 0xee: return {"xor  a,$", hex(lo, 2L)};
  case 0xef: return {"rst  $0028"};
  case 0xf0: return {"ldh  a,($ff", hex(lo, 2L), ")"};
  case 0xf1: return {"pop  af"};
  case 0xf2: return {"ldh  a,($ff00+c)"};
  case 0xf3: return {"di"};
  case 0xf5: return {"push af"};
  case 0xf6: return {"or  a,$", hex(lo, 2L)};
  case 0xf7: return {"rst  $0030"};
  case 0xf8: return {"ld   hl,sp+$", hex((int8)lo, 4L)};
  case 0xf9: return {"ld   sp,hl"};
  case 0xfa: return {"ld   a,($", hex(word, 4L), ")"};
  case 0xfb: return {"ei"};
  case 0xfe: return {"cp   a,$", hex(lo, 2L)};
  case 0xff: return {"rst  $0038"};
  }

  return {"xx"};
}

auto SM83::disassembleOpcodeCB(uint16 pc) -> string {
  auto opcode = readDebugger(pc);

  switch(opcode) {
  case 0x00: return {"rlc  b"};
  case 0x01: return {"rlc  c"};
  case 0x02: return {"rlc  d"};
  case 0x03: return {"rlc  e"};
  case 0x04: return {"rlc  h"};
  case 0x05: return {"rlc  l"};
  case 0x06: return {"rlc  (hl)"};
  case 0x07: return {"rlc  a"};
  case 0x08: return {"rrc  b"};
  case 0x09: return {"rrc  c"};
  case 0x0a: return {"rrc  d"};
  case 0x0b: return {"rrc  e"};
  case 0x0c: return {"rrc  h"};
  case 0x0d: return {"rrc  l"};
  case 0x0e: return {"rrc  (hl)"};
  case 0x0f: return {"rrc  a"};
  case 0x10: return {"rl   b"};
  case 0x11: return {"rl   c"};
  case 0x12: return {"rl   d"};
  case 0x13: return {"rl   e"};
  case 0x14: return {"rl   h"};
  case 0x15: return {"rl   l"};
  case 0x16: return {"rl   (hl)"};
  case 0x17: return {"rl   a"};
  case 0x18: return {"rr   b"};
  case 0x19: return {"rr   c"};
  case 0x1a: return {"rr   d"};
  case 0x1b: return {"rr   e"};
  case 0x1c: return {"rr   h"};
  case 0x1d: return {"rr   l"};
  case 0x1e: return {"rr   (hl)"};
  case 0x1f: return {"rr   a"};
  case 0x20: return {"sla  b"};
  case 0x21: return {"sla  c"};
  case 0x22: return {"sla  d"};
  case 0x23: return {"sla  e"};
  case 0x24: return {"sla  h"};
  case 0x25: return {"sla  l"};
  case 0x26: return {"sla  (hl)"};
  case 0x27: return {"sla  a"};
  case 0x28: return {"sra  b"};
  case 0x29: return {"sra  c"};
  case 0x2a: return {"sra  d"};
  case 0x2b: return {"sra  e"};
  case 0x2c: return {"sra  h"};
  case 0x2d: return {"sra  l"};
  case 0x2e: return {"sra  (hl)"};
  case 0x2f: return {"sra  a"};
  case 0x30: return {"swap b"};
  case 0x31: return {"swap c"};
  case 0x32: return {"swap d"};
  case 0x33: return {"swap e"};
  case 0x34: return {"swap h"};
  case 0x35: return {"swap l"};
  case 0x36: return {"swap (hl)"};
  case 0x37: return {"swap a"};
  case 0x38: return {"srl  b"};
  case 0x39: return {"srl  c"};
  case 0x3a: return {"srl  d"};
  case 0x3b: return {"srl  e"};
  case 0x3c: return {"srl  h"};
  case 0x3d: return {"srl  l"};
  case 0x3e: return {"srl  (hl)"};
  case 0x3f: return {"srl  a"};
  case 0x40: return {"bit  0,b"};
  case 0x41: return {"bit  0,c"};
  case 0x42: return {"bit  0,d"};
  case 0x43: return {"bit  0,e"};
  case 0x44: return {"bit  0,h"};
  case 0x45: return {"bit  0,l"};
  case 0x46: return {"bit  0,(hl)"};
  case 0x47: return {"bit  0,a"};
  case 0x48: return {"bit  1,b"};
  case 0x49: return {"bit  1,c"};
  case 0x4a: return {"bit  1,d"};
  case 0x4b: return {"bit  1,e"};
  case 0x4c: return {"bit  1,h"};
  case 0x4d: return {"bit  1,l"};
  case 0x4e: return {"bit  1,(hl)"};
  case 0x4f: return {"bit  1,a"};
  case 0x50: return {"bit  2,b"};
  case 0x51: return {"bit  2,c"};
  case 0x52: return {"bit  2,d"};
  case 0x53: return {"bit  2,e"};
  case 0x54: return {"bit  2,h"};
  case 0x55: return {"bit  2,l"};
  case 0x56: return {"bit  2,(hl)"};
  case 0x57: return {"bit  2,a"};
  case 0x58: return {"bit  3,b"};
  case 0x59: return {"bit  3,c"};
  case 0x5a: return {"bit  3,d"};
  case 0x5b: return {"bit  3,e"};
  case 0x5c: return {"bit  3,h"};
  case 0x5d: return {"bit  3,l"};
  case 0x5e: return {"bit  3,(hl)"};
  case 0x5f: return {"bit  3,a"};
  case 0x60: return {"bit  4,b"};
  case 0x61: return {"bit  4,c"};
  case 0x62: return {"bit  4,d"};
  case 0x63: return {"bit  4,e"};
  case 0x64: return {"bit  4,h"};
  case 0x65: return {"bit  4,l"};
  case 0x66: return {"bit  4,(hl)"};
  case 0x67: return {"bit  4,a"};
  case 0x68: return {"bit  5,b"};
  case 0x69: return {"bit  5,c"};
  case 0x6a: return {"bit  5,d"};
  case 0x6b: return {"bit  5,e"};
  case 0x6c: return {"bit  5,h"};
  case 0x6d: return {"bit  5,l"};
  case 0x6e: return {"bit  5,(hl)"};
  case 0x6f: return {"bit  5,a"};
  case 0x70: return {"bit  6,b"};
  case 0x71: return {"bit  6,c"};
  case 0x72: return {"bit  6,d"};
  case 0x73: return {"bit  6,e"};
  case 0x74: return {"bit  6,h"};
  case 0x75: return {"bit  6,l"};
  case 0x76: return {"bit  6,(hl)"};
  case 0x77: return {"bit  6,a"};
  case 0x78: return {"bit  7,b"};
  case 0x79: return {"bit  7,c"};
  case 0x7a: return {"bit  7,d"};
  case 0x7b: return {"bit  7,e"};
  case 0x7c: return {"bit  7,h"};
  case 0x7d: return {"bit  7,l"};
  case 0x7e: return {"bit  7,(hl)"};
  case 0x7f: return {"bit  7,a"};
  case 0x80: return {"res  0,b"};
  case 0x81: return {"res  0,c"};
  case 0x82: return {"res  0,d"};
  case 0x83: return {"res  0,e"};
  case 0x84: return {"res  0,h"};
  case 0x85: return {"res  0,l"};
  case 0x86: return {"res  0,(hl)"};
  case 0x87: return {"res  0,a"};
  case 0x88: return {"res  1,b"};
  case 0x89: return {"res  1,c"};
  case 0x8a: return {"res  1,d"};
  case 0x8b: return {"res  1,e"};
  case 0x8c: return {"res  1,h"};
  case 0x8d: return {"res  1,l"};
  case 0x8e: return {"res  1,(hl)"};
  case 0x8f: return {"res  1,a"};
  case 0x90: return {"res  2,b"};
  case 0x91: return {"res  2,c"};
  case 0x92: return {"res  2,d"};
  case 0x93: return {"res  2,e"};
  case 0x94: return {"res  2,h"};
  case 0x95: return {"res  2,l"};
  case 0x96: return {"res  2,(hl)"};
  case 0x97: return {"res  2,a"};
  case 0x98: return {"res  3,b"};
  case 0x99: return {"res  3,c"};
  case 0x9a: return {"res  3,d"};
  case 0x9b: return {"res  3,e"};
  case 0x9c: return {"res  3,h"};
  case 0x9d: return {"res  3,l"};
  case 0x9e: return {"res  3,(hl)"};
  case 0x9f: return {"res  3,a"};
  case 0xa0: return {"res  4,b"};
  case 0xa1: return {"res  4,c"};
  case 0xa2: return {"res  4,d"};
  case 0xa3: return {"res  4,e"};
  case 0xa4: return {"res  4,h"};
  case 0xa5: return {"res  4,l"};
  case 0xa6: return {"res  4,(hl)"};
  case 0xa7: return {"res  4,a"};
  case 0xa8: return {"res  5,b"};
  case 0xa9: return {"res  5,c"};
  case 0xaa: return {"res  5,d"};
  case 0xab: return {"res  5,e"};
  case 0xac: return {"res  5,h"};
  case 0xad: return {"res  5,l"};
  case 0xae: return {"res  5,(hl)"};
  case 0xaf: return {"res  5,a"};
  case 0xb0: return {"res  6,b"};
  case 0xb1: return {"res  6,c"};
  case 0xb2: return {"res  6,d"};
  case 0xb3: return {"res  6,e"};
  case 0xb4: return {"res  6,h"};
  case 0xb5: return {"res  6,l"};
  case 0xb6: return {"res  6,(hl)"};
  case 0xb7: return {"res  6,a"};
  case 0xb8: return {"res  7,b"};
  case 0xb9: return {"res  7,c"};
  case 0xba: return {"res  7,d"};
  case 0xbb: return {"res  7,e"};
  case 0xbc: return {"res  7,h"};
  case 0xbd: return {"res  7,l"};
  case 0xbe: return {"res  7,(hl)"};
  case 0xbf: return {"res  7,a"};
  case 0xc0: return {"set  0,b"};
  case 0xc1: return {"set  0,c"};
  case 0xc2: return {"set  0,d"};
  case 0xc3: return {"set  0,e"};
  case 0xc4: return {"set  0,h"};
  case 0xc5: return {"set  0,l"};
  case 0xc6: return {"set  0,(hl)"};
  case 0xc7: return {"set  0,a"};
  case 0xc8: return {"set  1,b"};
  case 0xc9: return {"set  1,c"};
  case 0xca: return {"set  1,d"};
  case 0xcb: return {"set  1,e"};
  case 0xcc: return {"set  1,h"};
  case 0xcd: return {"set  1,l"};
  case 0xce: return {"set  1,(hl)"};
  case 0xcf: return {"set  1,a"};
  case 0xd0: return {"set  2,b"};
  case 0xd1: return {"set  2,c"};
  case 0xd2: return {"set  2,d"};
  case 0xd3: return {"set  2,e"};
  case 0xd4: return {"set  2,h"};
  case 0xd5: return {"set  2,l"};
  case 0xd6: return {"set  2,(hl)"};
  case 0xd7: return {"set  2,a"};
  case 0xd8: return {"set  3,b"};
  case 0xd9: return {"set  3,c"};
  case 0xda: return {"set  3,d"};
  case 0xdb: return {"set  3,e"};
  case 0xdc: return {"set  3,h"};
  case 0xdd: return {"set  3,l"};
  case 0xde: return {"set  3,(hl)"};
  case 0xdf: return {"set  3,a"};
  case 0xe0: return {"set  4,b"};
  case 0xe1: return {"set  4,c"};
  case 0xe2: return {"set  4,d"};
  case 0xe3: return {"set  4,e"};
  case 0xe4: return {"set  4,h"};
  case 0xe5: return {"set  4,l"};
  case 0xe6: return {"set  4,(hl)"};
  case 0xe7: return {"set  4,a"};
  case 0xe8: return {"set  5,b"};
  case 0xe9: return {"set  5,c"};
  case 0xea: return {"set  5,d"};
  case 0xeb: return {"set  5,e"};
  case 0xec: return {"set  5,h"};
  case 0xed: return {"set  5,l"};
  case 0xee: return {"set  5,(hl)"};
  case 0xef: return {"set  5,a"};
  case 0xf0: return {"set  6,b"};
  case 0xf1: return {"set  6,c"};
  case 0xf2: return {"set  6,d"};
  case 0xf3: return {"set  6,e"};
  case 0xf4: return {"set  6,h"};
  case 0xf5: return {"set  6,l"};
  case 0xf6: return {"set  6,(hl)"};
  case 0xf7: return {"set  6,a"};
  case 0xf8: return {"set  7,b"};
  case 0xf9: return {"set  7,c"};
  case 0xfa: return {"set  7,d"};
  case 0xfb: return {"set  7,e"};
  case 0xfc: return {"set  7,h"};
  case 0xfd: return {"set  7,l"};
  case 0xfe: return {"set  7,(hl)"};
  case 0xff: return {"set  7,a"};
  }

  unreachable;
}
