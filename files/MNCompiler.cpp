#include "MNCompiler.h"
#include "MNFunction.h"
#include "MNObject.h"

MNFuncBuilder::MNFuncBuilder(MNFuncBuilder* up)
	: upFunc(up)
	, func(new MNFunction())
	, codeMaker(func->m_codes, func->m_ncode)
{
}

tsize MNFuncBuilder::addConst(const MNObject& val)
{
	func->m_consts.push_back(val);
	return func->m_consts.size() - 1;
}