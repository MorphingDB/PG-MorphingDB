#include "postgres.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


#include "commands/mdcommands.h"
#include "model/libtorch_wrapper.h"



void
createmd(ParseState *pstate, const CreatemdStmt *stmt)
{
    // todo
    if(!loadTorchModel(stmt->mdname, stmt->modelPath)) {
        ereport(ERROR,
				(errcode(ERRCODE_MODEL_ERROE),
				 errmsg("model \"%s\" create failed", stmt->mdname)));
    }
}