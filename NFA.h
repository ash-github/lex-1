#ifndef _YANJUN_NFA_H_
#define _YANJUN_NFA_H_

#include <cstdlib>
#include <list>
#include <stack>
using namespace std;

#include "Limits.h"
#include "State.h"
#include "AuxNFAPart.h"
#include "NFAFrag.h"

#include "lexicaldesc.h"

class NFA:public NFAPart
{
public:
	NFA();

	~NFA();

	void build(LexicalDesc& lexDesc);

	NFAState* nfaStartStates[STARTSTATE_MAX_COUNT];
};

#endif//_YANJUN_NFA_H_