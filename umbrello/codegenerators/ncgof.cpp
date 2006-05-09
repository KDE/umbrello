#include "ncgof.h"

#include "cppheadercodedocument.h"
#include "cppsourcecodedocument.h"
#include "javaclassifiercodedocument.h"
#include "rubyclassifiercodedocument.h"

#include "cppheadercodeoperation.h"
#include "cppsourcecodeoperation.h"
#include "javacodeoperation.h"
#include "rubycodeoperation.h"
#include "../uml.h"

namespace NCGOF {

CodeOperation *newCodeOperation(ClassifierCodeDocument *ccd, UMLOperation * op) {
    CodeOperation *retval = NULL;
    switch (UMLApp::app()->getActiveLanguage()) {
        case Uml::pl_Cpp:
            {
                CPPHeaderCodeDocument *hcd = dynamic_cast<CPPHeaderCodeDocument*>(ccd);
                if (hcd)
                    retval = new CPPHeaderCodeOperation(hcd, op);
                else
                    retval = new CPPSourceCodeOperation(dynamic_cast<CPPSourceCodeDocument*>(ccd), op);
            }
            break;
        case Uml::pl_Java:
            retval = new JavaCodeOperation(dynamic_cast<JavaClassifierCodeDocument*>(ccd), op);
            break;
        case Uml::pl_Ruby:
            retval = new RubyCodeOperation(dynamic_cast<RubyClassifierCodeDocument*>(ccd), op);
            break;
        default:
            break;
    }
    return retval;
}

}  // end namespace NCGOF
