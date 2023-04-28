#ifndef MDCOMMANDS_H
#define MDCOMMANDS_H


#include "access/xlogreader.h"
#include "catalog/objectaddress.h"
#include "lib/stringinfo.h"
#include "nodes/parsenodes.h"

extern void createmd(ParseState *pstate, const CreatemdStmt *stmt);
extern void dropmd(ParseState *pstate, const DropmdStmt *stmt);
extern void updatemd(ParseState *pstate, const UpdatemdStmt *stmt);

#endif							/* MDCOMMANDS_H */