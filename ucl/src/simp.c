#include "ucl.h"
#include "gen.h"
 /**
  * if u > 1 and u == 2 power of n, return;
  * otherwise, return 0
  */
static int Power2(unsigned int u)
{
    int n;

    if (u > 1 && (u &(u - 1)) == 0)
    {
        for (n = 0; u; u >>= 1, n++)
        {
            if (u & 1)
                return n;
        }
    }
    return 0;
}
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
            // a - 0 = a;
            if (src2->kind == SK_Constant && src2->val.i[0] == 0)
                return src1;
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
        case MUL:
        case DIV:
            // a * 1 = a; a / 1 = a;                                                                     
            if (src2->val.i[0] == 1)                                                                     
                return src1;                                                                             
                                                                                                      
            // a * 2 power of n = a >> n                                                                 
            c1 = Power2(src2->val.i[0]);                                                                 
            if (c1 != 0)                                                                                 
            {                                                                                            
                src2 = IntConstant(c1);                                                                  
                opcode = opcode == MUL ? LSH : RSH;                                                      
            }                                                                                            
            break;
        case BOR:
            // a | 0 = a; a | -1 = -1
            if (src2->val.i[0] == 0)
                return src1;
            if (src2->val.i[0] == -1)
                return src2;
            break;
        case BXOR:
            // a ^ 0 = a
            if (src2->val.i[0] == 0)
                return src1;
            break;
        case BAND:
            // a & 0 = 0, a & -1 = a
            if (src2->val.i[0] == 0)
                return IntConstant(0);
            if (src2->val.i[0] == -1)
                return src1;
            break;
        default:
            break;
    }
add_value:
	return TryAddValue(ty, opcode, src1, src2);    
}