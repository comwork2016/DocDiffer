#ifndef DOCUMENTOPERATION_H
#define DOCUMENTOPERATION_H

#include <string>
#include <algorithm>
#include "../content/Document.h"
#include "../content/Document.h"
#include "../util/NLPIRUtil.h"
#include "LongestSimilarSentence.h"

class DocumentOperation
{
    public:
        DocumentOperation();
        virtual ~DocumentOperation();

        static void GetDiffer(Document* docOrigin,Document* docSearch);
    protected:
    private:
};

#endif // DOCUMENTOPERATION_H
