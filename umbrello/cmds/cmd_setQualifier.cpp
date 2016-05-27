#include "cmd_setQualifier.h"

// app includes
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"

#include <KLocalizedString>

namespace Uml
{

    CmdSetQualifier::CmdSetQualifier(UMLObject * obj, Uml::TypeQualifiers::Enum qualifier)
      : CmdBaseObjectCommand(obj),
        m_qualifier(qualifier)
    {
        setText(i18n("Change qualifier : %1", obj->name()));
        m_oldQualifier = obj->qualifier();
    }

    void CmdSetQualifier::redo()
    {
        UMLObject *umlObject = object();
        if (umlObject)
            umlObject->setQualifiersCmd(m_qualifier);
    }

    void CmdSetQualifier::undo()
    {
        UMLObject *umlObject = object();
        if (umlObject)
            umlObject->setQualifiersCmd(m_oldQualifier);
    }

}
