#ifndef IMPORT_ROSE__H
#define IMPORT_ROSE__H

#include <qiodevice.h>

/**
 * Rose model import
 *
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
namespace Import_Rose {

    /**
     * Parse a file into the PetalNode internal tree representation
     * and then create Umbrello objects by traversing the tree.
     *
     * @return  True for success, false in case of error.
     */
    bool loadFromMDL(QIODevice & file);

}

#endif
