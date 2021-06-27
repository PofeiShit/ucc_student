#ifndef __REG_H_
#define __REG_H_
enum { EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI };
void StoreVar(Symbol reg, Symbol v);
void SpillReg(Symbol reg);

extern Symbol X86Regs[];
extern Symbol X86ByteRegs[];
#endif
