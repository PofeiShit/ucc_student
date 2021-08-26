#include "ucl.h"
#include "gen.h"
/**
 * Perform algebraic simplification and strenth reduction
 */
Symbol Simplify(Type ty, int opcode, Symbol src1, Symbol src2)
{
    switch(opcode)
    {
        case BOR:
            // a | 0 = a; a | -1 = -1
            if (src2->val.i[0] == 0)
                return src1;
            if (src2->val.i[0] == -1)
                return src2;
            break;
    }
	return TryAddValue(ty, opcode, src1, src2);    
}