#include "ucl.h"
#include "gen.h"
/**
 * Perform algebraic simplification and strenth reduction
 */
Symbol Simplify(Type ty, int opcode, Symbol src1, Symbol src2)
{
    Symbol p1, p2;
    int c1, c2;
    // src2 为空，或者为常量,或者a-a三种情况可以简化
    if (src2 == NULL || (src2->kind != SK_Constant && opcode != SUB)) 
        goto add_value;
    switch(opcode)
    {
        case ADD:
            if (src2->val.i[0] == 0)
                return src1;
            break;
        case SUB:
		// put source operand into v + c format (v maybe NULL, c maybe 0)
		    p1 = src1; c1 = 0;

            p2 = src2; c2 = 0;
            if (src2->kind == SK_Constant) {
                p2 = NULL;
                c2 = src2->val.i[0];
            }
            if (p2 == NULL) {
                src1 = p1;
                opcode = ADD;
                src2 = IntConstant(c1 - c2);
            }
            break;
        case BOR:
            // a | 0 = a; a | -1 = -1
            if (src2->val.i[0] == 0)
                return src1;
            if (src2->val.i[0] == -1)
                return src2;
            break;
        default:
            break;
    }
add_value:
	return TryAddValue(ty, opcode, src1, src2);    
}