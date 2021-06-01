#include "ucl.h"
#include "gen.h"

/**
 * Add a predecessor p to basic block bb
 */
static void AddPredecessor(BBlock bb, BBlock p)
{
	CFGEdge e;

	ALLOC(e);
	e->bb = p;
	e->next = bb->preds;
	bb->preds = e;
	bb->npred++;
}

/**
 * Add a successor s to basic block bb
 */ 
static void AddSuccessor(BBlock bb, BBlock s)
{
	CFGEdge e;

	ALLOC(e);
	e->bb = s;
	e->next = bb->succs;
	bb->succs = e;
	bb->nsucc++;
}
void DrawCFGEdge(BBlock head, BBlock tail)
{
	AddSuccessor(head, tail);
	AddPredecessor(tail, head);
}