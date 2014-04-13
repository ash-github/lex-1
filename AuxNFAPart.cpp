#include "AuxNFAPart.h"

void AuxNFAPart::addPart(const char* n,const char* r)
{
	_AuxNFAPart* newpart=new _AuxNFAPart(n,r);
	newpart->nfapart.fromRE(r,*this);
	parts.push_back(newpart);
}

NFAPart* AuxNFAPart::searchPart(const char* n)
{
	list<_AuxNFAPart*>::iterator it=parts.begin();
	for(;it!=parts.end();it++)
	{
		if(strcmp(n,(*it)->name)==0)
			return &((*it)->nfapart);
	}
	return NULL;		
}

void AuxNFAPart::clear()
{
	list<_AuxNFAPart*>::iterator it=parts.begin();
	for(;it!=parts.end();it++)
	{
		delete *it;
	}
	parts.clear();
}