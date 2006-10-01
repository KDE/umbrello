
#include "uniqueid.h"

# define EXTERNALIZE_ID(id)  QString::number(id).ascii()

namespace UniqueID {

/**
 * Each model object gets assigned a unique ID.
 * NOTE: Currently this is an int although Uml::IDType is a string.
 *       See also UMLView::m_nLocalID.
 */
int m_uniqueID = 0;

Uml::IDType gen() {
    ++m_uniqueID;
    return EXTERNALIZE_ID(m_uniqueID);
}

void init() {
    m_uniqueID = 0;
}

Uml::IDType get() {
    return EXTERNALIZE_ID(m_uniqueID);
}

void set(Uml::IDType id) {
    QString uniqueid = ID2STR(id);
    m_uniqueID = uniqueid.toInt();
}

}  // end namespace UniqueID

