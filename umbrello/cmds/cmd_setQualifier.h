#ifndef CMDSETQUALIFIER_H
#define CMDSETQUALIFIER_H

#include "cmd_baseObjectCommand.h"

namespace Uml{

    class CmdSetQualifier : public CmdBaseObjectCommand
    {
        public:
            CmdSetQualifier(UMLObject* obj, TypeQualifiers::Enum qualifier);

            void redo();
            void undo();

        private:
            Uml::TypeQualifiers::Enum m_oldQualifier;
            Uml::TypeQualifiers::Enum m_qualifier;
    };


}

#endif // CMDSETQUALIFIER_H
