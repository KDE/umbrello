/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "assocrules.h"

// local includes
#include "debug_utils.h"
#include "uml.h"
#include "umlview.h"
#include "umlwidget.h"
#include "umlobject.h"
#include "associationwidgetlist.h"
#include "associationwidget.h"
#include "statewidget.h"
#include "activitywidget.h"
#include "signalwidget.h"
#include "forkjoinwidget.h"
#include "umlscene.h"
#include "umllistview.h"

// kde includes
#include <typeinfo>
#include <KMessageBox>

/**
 * Constructor.
 */
AssocRules::AssocRules()
{
}

/**
 * Destructor.
 */
AssocRules::~AssocRules()
{
}

/**
 * Returns whether an association is going to be allowed for the given
 * values. This method is used to test if you can start an association.
 */
bool AssocRules::allowAssociation(Uml::AssociationType::Enum assocType, UMLWidget * widget)
{
    WidgetBase::WidgetType widgetType = widget->baseType();
    bool bValid = false;
    for (int i = 0; i < m_nNumRules; ++i) {
        const Assoc_Rule& rule = m_AssocRules[i];
        if (!rule.isValid())
            continue;
        if (assocType != rule.assoc_type)
            continue;
        if (widgetType == rule.widgetA_type ||
            (widgetType == rule.widgetB_type && rule.bidirectional)) {
            bValid =  true;
            break;
        }
    }
    if(!bValid) {
        // Special case: Subsystem realizes interface in component diagram
        UMLView *view = UMLApp::app()->currentView();
        if (view && view->umlScene()->isComponentDiagram() &&
            widgetType == WidgetBase::wt_Package &&
                (assocType == Uml::AssociationType::Generalization ||
                    assocType == Uml::AssociationType::Realization)) {
            uDebug() << "allowAssociation(widget " << widgetType << ", assoc " << assocType
                     << ") : Exception for subsystem realizing interface in component diagram";
        } else {
            uDebug() << "allowAssociation(widget " << widgetType << ", assoc " << assocType
                     << ") : no permission rule found";
            return false;
        }
    }
    AssociationWidgetList list = widget->associationWidgetList();

    switch(assocType) {
    case Uml::AssociationType::Association:
    case Uml::AssociationType::UniAssociation:
    case Uml::AssociationType::Dependency:
    case Uml::AssociationType::Coll_Mesg_Sync:
    case Uml::AssociationType::Coll_Mesg_Async:
    case Uml::AssociationType::Generalization://can have many sub/super types
    case Uml::AssociationType::Aggregation:
    case Uml::AssociationType::Relationship:
    case Uml::AssociationType::Composition:
    case Uml::AssociationType::Containment:
        return true;//doesn't matter whats already connected to widget
        break;

    case Uml::AssociationType::Association_Self:
        return true;// we should really check that connection is to same object
        break;

    case Uml::AssociationType::Realization:  // one connected to widget only (a or b)
        foreach (AssociationWidget* assoc, list) {
            if (assoc->associationType() == Uml::AssociationType::Realization) {
                uDebug() << "allowAssociation(widget " << widgetType << ", assoc " << assocType
                         << ") : disallowing more than one realization to object";
                return false;
            }
        }
        return true;
        break;

    case Uml::AssociationType::State:
        {
            StateWidget *pState = widget->asStateWidget();
            if (pState == 0 || pState->stateType() != StateWidget::End)
                return true;
            uDebug() << "allowAssociation(widget " << widgetType << ", assoc " << assocType
                     << ") : disallowing because state type is not 'End'";
        }
        break;

    case Uml::AssociationType::Activity:
    case Uml::AssociationType::Exception:
        {
            ActivityWidget *pActivity = widget->asActivityWidget();
            if (pActivity == 0 || pActivity->activityType() != ActivityWidget::End)
                return true;
            uDebug() << "allowAssociation(widget " << widgetType << ", assoc " << assocType
                     << ") : disallowing because activity type is not 'End'";
        }
        break;

    case Uml::AssociationType::Anchor:
        return true;
        break;

    case Uml::AssociationType::Category2Parent:
        if (widgetType == WidgetBase::wt_Category)
            return true;
        uDebug() << "allowAssociation(widget " << widgetType << ", assoc " << assocType
                 << ") : disallowing because widget is not Category";
        break;

    case Uml::AssociationType::Child2Category:
        if (widgetType == WidgetBase::wt_Entity)
            return true;
        uDebug() << "allowAssociation(widget " << widgetType << ", assoc " << assocType
                 << ") : disallowing because widget is not Entity";
        break;

    default:
        uWarning() << "allowAssociation() on unknown type";
        break;
    }
    return false;
}

/**
 * Returns whether an association is valid with the given variables.
 * This method is used to finish an association.
 * When we know what we are going to connect both ends of the association to, we can
 * use this method.
 */
bool AssocRules::allowAssociation(Uml::AssociationType::Enum assocType,
                                   UMLWidget * widgetA, UMLWidget * widgetB)
{
    WidgetBase::WidgetType widgetTypeA = widgetA->baseType();
    WidgetBase::WidgetType widgetTypeB = widgetB->baseType();
    bool bValid = false;

    if (widgetA->umlObject() && widgetA->umlObject() == widgetB->umlObject()) {
        return allowSelf(assocType, widgetTypeA);
    }

    for (int i = 0; i < m_nNumRules; ++i) {
        const Assoc_Rule& rule = m_AssocRules[i];
        if (!rule.isValid())
            continue;
        if (assocType != rule.assoc_type)
            continue;
        if ((widgetTypeA == rule.widgetA_type &&
             widgetTypeB == rule.widgetB_type) ||
             (rule.bidirectional &&
              widgetTypeB == rule.widgetA_type &&
              widgetTypeA == rule.widgetB_type)) {
            bValid = true;
            break;
        }
    }

    if (!bValid) {
        return false;
    }

    //Prevent against a package containing its own parent! #packageception.
    if (assocType == Uml::AssociationType::Containment) {
          UMLListViewItem* listItemA = UMLApp::app()->listView()->findUMLObject(widgetA->umlObject());
          UMLListViewItem* listItemB = UMLApp::app()->listView()->findUMLObject(widgetB->umlObject());

          if (listItemA && listItemB) {
              // Great, we have our listviewitems, now check to make sure that they don't become recursive.
              if (listItemA->parent() == static_cast<QTreeWidgetItem*>(listItemB)) {
                  // The user is trying to make the parent the child and the child the parent. Stop them!
                  return false;
              }
          }
          // This was just a little assertion for safety, don't return yet!
    }

    AssociationWidgetList list = widgetB->associationWidgetList();

    switch(assocType) {
    case Uml::AssociationType::Association_Self:
        if (widgetA->umlObject() == widgetB->umlObject())
            return true;
        break;

    case Uml::AssociationType::Association:
    case Uml::AssociationType::UniAssociation:
    case Uml::AssociationType::Dependency:
    case Uml::AssociationType::Coll_Mesg_Sync:
    case Uml::AssociationType::Coll_Mesg_Async:
    case Uml::AssociationType::Aggregation:
    case Uml::AssociationType::Relationship:
        return true;  // doesn't matter what's already connected to widget
        break;

    case Uml::AssociationType::Composition:
    case Uml::AssociationType::Containment:
        return true;
        break;

    case Uml::AssociationType::Generalization://can have many sub/super types but can't sup/sub each
        foreach (AssociationWidget * assoc, list) {
            if((widgetA == assoc->widgetForRole(Uml::RoleType::A) ||
                        widgetA == assoc->widgetForRole(Uml::RoleType::B))
                    && assoc->associationType() == assocType)
                return false;
        }
        return true;
        break;

    case Uml::AssociationType::Realization: // can only connect to abstract (interface) classes
        foreach(AssociationWidget * assoc, list) {
            if((widgetA == assoc->widgetForRole(Uml::RoleType::A) ||
                        widgetA == assoc->widgetForRole(Uml::RoleType::B))
                    && assoc->associationType() == Uml::AssociationType::Realization) {
                return false;
            }
        }
        if (widgetB->isClassWidget()) {
            return widgetB->umlObject()->isAbstract();
        } else if (widgetB->isInterfaceWidget() ||
                   widgetB->isPackageWidget()) {
            return true;
        }
        break;

    case Uml::AssociationType::State:
        {
            StateWidget *stateA = widgetA->asStateWidget();
            StateWidget *stateB = widgetB->asStateWidget();
            if (stateA && stateB) {
                if (stateB->stateType() == StateWidget::Initial)
                    return false;
                if (stateB->stateType() == StateWidget::End &&
                    stateA->stateType() != StateWidget::Normal)
                    return false;
            }
        }
        return true;
        break;

    case Uml::AssociationType::Activity:
    case Uml::AssociationType::Exception:
        {

            ActivityWidget *actA = widgetA->asActivityWidget();
            ActivityWidget *actB = widgetB->asActivityWidget();

            bool isSignal = false;
            bool isObjectNode = false;

            if (widgetTypeA == WidgetBase::wt_Signal)
                isSignal = true;
            else if (widgetTypeA == WidgetBase::wt_ObjectNode)
                isObjectNode = true;

            // no transitions to initial activity allowed
            if (actB && actB->activityType() == ActivityWidget::Initial) {
                return false;
            }
            // actType -1 here means "not applicable".
            int actTypeA = -1;
            if (actA)
                actTypeA = actA->activityType();
            int actTypeB = -1;
            if (actB)
                actTypeB = actB->activityType();
            // only from a signalwidget, an objectnode widget, a normal activity, branch or fork activity, to the end
            if ((actTypeB == ActivityWidget::End || actTypeB == ActivityWidget::Final) &&
                actTypeA != ActivityWidget::Normal &&
                actTypeA != ActivityWidget::Branch &&
                widgetA->asForkJoinWidget() == 0 && !isSignal &&!isObjectNode) {
                return false;
            }
            // only Forks and Branches can have more than one "outgoing" transition
            if (actA != 0 && actTypeA != ActivityWidget::Branch) {
                AssociationWidgetList list = widgetA->associationWidgetList();
                foreach (AssociationWidget* assoc, list) {
                    if (assoc->widgetForRole(Uml::RoleType::A) == widgetA) {
                        return false;
                    }
                }
            }
        }
        return true;
        break;

    case Uml::AssociationType::Anchor:
        return true;
        break;

    case Uml::AssociationType::Category2Parent:
        if (widgetTypeA == WidgetBase::wt_Category && widgetTypeB == WidgetBase::wt_Entity) {
            return true;
        }
        break;

    case Uml::AssociationType::Child2Category:
        if (widgetTypeA == WidgetBase::wt_Entity && widgetTypeB == WidgetBase::wt_Category) {
            return true;
        }
        break;

    default:
        uWarning() << "allowAssociation() on unknown type";
        break;
    }
    return false;
}

/**
 * Returns whether to allow a role text for the given association type.
 */
bool AssocRules::allowRole(Uml::AssociationType::Enum assocType)
{
    for(int i = 0; i < m_nNumRules; ++i) {
        const Assoc_Rule& rule = m_AssocRules[i];
        if (!rule.isValid())
            continue;
        if(assocType == rule.assoc_type)
            return rule.role;
    }
    return false;
}

/**
 * Returns whether to allow a multiplicity text for the given
 * association and widget type.
 */
bool AssocRules::allowMultiplicity(Uml::AssociationType::Enum assocType, WidgetBase::WidgetType widgetType)
{
    for(int i = 0; i < m_nNumRules; ++i) {
        const Assoc_Rule& rule = m_AssocRules[i];
        if (!rule.isValid())
            continue;
        if(assocType == rule.assoc_type)
            if(widgetType == rule.widgetA_type || widgetType == rule.widgetB_type)
                return rule.multiplicity;
    }
    return false;
}

/**
 * Returns whether to allow an association to self for given variables.
 */
bool AssocRules::allowSelf(Uml::AssociationType::Enum assocType, WidgetBase::WidgetType widgetType)
{
    for(int i = 0; i < m_nNumRules; ++i) {
        const Assoc_Rule& rule = m_AssocRules[i];
        if (!rule.isValid())
            continue;
        if(assocType == rule.assoc_type)
            if(widgetType == rule.widgetA_type || widgetType == rule.widgetB_type)
                return rule.self;
    }
    return false;
}

/**
 * Returns whether an implements association should be a Realisation or
 * a Generalisation.
 * as defined in m_AssocRules.
 */
Uml::AssociationType::Enum AssocRules::isGeneralisationOrRealisation(UMLWidget* widgetA, UMLWidget* widgetB)
{
    WidgetBase::WidgetType widgetTypeA = widgetA->baseType();
    WidgetBase::WidgetType widgetTypeB = widgetB->baseType();
    for (int i = 0; i < m_nNumRules; ++i) {
        const Assoc_Rule &rule = m_AssocRules[i];
        if (!rule.isValid())
            continue;
        if (rule.assoc_type == Uml::AssociationType::Realization &&
                widgetTypeA == rule.widgetA_type &&
                widgetTypeB == rule.widgetB_type) {
            return Uml::AssociationType::Realization;
        }
    }
    return Uml::AssociationType::Generalization;
}

AssocRules::Assoc_Rule AssocRules::m_AssocRules[] = {
    // Language  Association                       widgetA                    widgetB                     role    multi   bidir.  self   level
    //----+---------------------------------------+--------------------------+---------------------------+-------+-------+-------+-----+----
    { All, Uml::AssociationType::Association_Self, WidgetBase::wt_Class,      WidgetBase::wt_Class,       true,   true,   true,   true,  Any  },
    { All, Uml::AssociationType::Association_Self, WidgetBase::wt_Object,     WidgetBase::wt_Object,      true,   true,   true,   true,  Any  },
    { All, Uml::AssociationType::Association_Self, WidgetBase::wt_Interface,  WidgetBase::wt_Interface,   true,   true,   true,   true,  Any  },
    { All, Uml::AssociationType::Association,      WidgetBase::wt_Class,      WidgetBase::wt_Class,       true,   true,   true,   true,  Any  },
    { All, Uml::AssociationType::Association,      WidgetBase::wt_Object,     WidgetBase::wt_Object,      true,   true,   true,   true,  Any  },
    { All, Uml::AssociationType::Association,      WidgetBase::wt_Instance,   WidgetBase::wt_Instance,    true,   true,   true,   true,  Any  },
    { All, Uml::AssociationType::Association,      WidgetBase::wt_Interface,  WidgetBase::wt_Interface,   true,   true,   true,   true,  Any  },
    { All, Uml::AssociationType::Association,      WidgetBase::wt_Class,      WidgetBase::wt_Interface,   true,   true,   true,   false, Any  },
    { All, Uml::AssociationType::Association,      WidgetBase::wt_Class,      WidgetBase::wt_Datatype,    true,   true,   true,   false, Any  },
    { All, Uml::AssociationType::Association,      WidgetBase::wt_Class,      WidgetBase::wt_Enum,        true,   true,   true,   false, Any  },
    { All, Uml::AssociationType::Association,      WidgetBase::wt_Actor,      WidgetBase::wt_UseCase,     true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::Association,      WidgetBase::wt_UseCase,    WidgetBase::wt_UseCase,     true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::Association,      WidgetBase::wt_Actor,      WidgetBase::wt_Actor,       true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::Association,      WidgetBase::wt_Actor,      WidgetBase::wt_UseCase,     true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::Association,      WidgetBase::wt_Component,  WidgetBase::wt_Interface,   true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::Association,      WidgetBase::wt_Port,       WidgetBase::wt_Interface,   true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::Association,      WidgetBase::wt_Interface,  WidgetBase::wt_Artifact,    true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::Association,      WidgetBase::wt_Interface,  WidgetBase::wt_Component,   true,   false,  false,  false, UML2 },
    { All, Uml::AssociationType::Association,      WidgetBase::wt_Interface,  WidgetBase::wt_Port,        true,   false,  false,  false, UML2 },
    { All, Uml::AssociationType::Association,      WidgetBase::wt_Node,       WidgetBase::wt_Node,        true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::Association,      WidgetBase::wt_Node,       WidgetBase::wt_Node,        true,   false,  false,  false, Any  },
    {Java, Uml::AssociationType::Association,      WidgetBase::wt_Enum,       WidgetBase::wt_Enum,        true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::UniAssociation,   WidgetBase::wt_Class,      WidgetBase::wt_Class,       true,   true,   true,   true,  Any  },
    { All, Uml::AssociationType::UniAssociation,   WidgetBase::wt_Object,     WidgetBase::wt_Object,      true,   true,   true,   true,  Any  },
    { All, Uml::AssociationType::UniAssociation,   WidgetBase::wt_Interface,  WidgetBase::wt_Interface,   true,   true,   true,   true,  Any  },
    { All, Uml::AssociationType::UniAssociation,   WidgetBase::wt_Class,      WidgetBase::wt_Interface,   true,   true,   true,   true,  Any  },
    { All, Uml::AssociationType::UniAssociation,   WidgetBase::wt_Class,      WidgetBase::wt_Datatype,    true,   true,   true,   true,  Any  },
    { All, Uml::AssociationType::UniAssociation,   WidgetBase::wt_Class,      WidgetBase::wt_Enum,        true,   true,   true,   true,  Any  },
    { All, Uml::AssociationType::UniAssociation,   WidgetBase::wt_Actor,      WidgetBase::wt_Actor,       true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::UniAssociation,   WidgetBase::wt_UseCase,    WidgetBase::wt_UseCase,     true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::UniAssociation,   WidgetBase::wt_UseCase,    WidgetBase::wt_Actor,       true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::UniAssociation,   WidgetBase::wt_Component,  WidgetBase::wt_Interface,   true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::UniAssociation,   WidgetBase::wt_Component,  WidgetBase::wt_Artifact,    true,   false,  true,   false, Any  },
    { All, Uml::AssociationType::UniAssociation,   WidgetBase::wt_Port,       WidgetBase::wt_Interface,   true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::UniAssociation,   WidgetBase::wt_Node,       WidgetBase::wt_Node,        true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::Generalization,   WidgetBase::wt_Class,      WidgetBase::wt_Datatype,    false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Generalization,   WidgetBase::wt_Class,      WidgetBase::wt_Class,       false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Generalization,   WidgetBase::wt_Interface,  WidgetBase::wt_Interface,   false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Generalization,   WidgetBase::wt_Interface,  WidgetBase::wt_Class,       false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Generalization,   WidgetBase::wt_UseCase,    WidgetBase::wt_UseCase,     false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Generalization,   WidgetBase::wt_Actor,      WidgetBase::wt_Actor,       false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Generalization,   WidgetBase::wt_Component,  WidgetBase::wt_Interface,   false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Aggregation,      WidgetBase::wt_Class,      WidgetBase::wt_Class,       true,   true,   false,  true,  Any  },
    { All, Uml::AssociationType::Aggregation,      WidgetBase::wt_Class,      WidgetBase::wt_Interface,   true,   true,   false,  false, Any  },
    { All, Uml::AssociationType::Aggregation,      WidgetBase::wt_Class,      WidgetBase::wt_Enum,        true,   true,   false,  false, Any  },
    { All, Uml::AssociationType::Aggregation,      WidgetBase::wt_Class,      WidgetBase::wt_Datatype,    true,   true,   false,  false, Any  },
    { All, Uml::AssociationType::Dependency,       WidgetBase::wt_Class,      WidgetBase::wt_Class,       true,   false,  false,  true,  Any  },
    { All, Uml::AssociationType::Dependency,       WidgetBase::wt_UseCase,    WidgetBase::wt_UseCase,     true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::Dependency,       WidgetBase::wt_Actor,      WidgetBase::wt_Actor,       true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::Dependency,       WidgetBase::wt_Actor,      WidgetBase::wt_UseCase,     true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::Dependency,       WidgetBase::wt_Package,    WidgetBase::wt_Package,     true,   true,   true,   false, Any  },
    { All, Uml::AssociationType::Dependency,       WidgetBase::wt_Class,      WidgetBase::wt_Package,     true,   true,   true,   false, Any  },
    { All, Uml::AssociationType::Dependency,       WidgetBase::wt_Interface,  WidgetBase::wt_Package,     true,   true,   true,   false, Any  },
    { All, Uml::AssociationType::Dependency,       WidgetBase::wt_Interface,  WidgetBase::wt_Interface,   true,   true,   true,   true,  Any  },
    { All, Uml::AssociationType::Dependency,       WidgetBase::wt_Class,      WidgetBase::wt_Interface,   true,   true,   true,   false, Any  },
    { All, Uml::AssociationType::Dependency,       WidgetBase::wt_Class,      WidgetBase::wt_Datatype,    true,   true,   true,   false, Any  },
    { All, Uml::AssociationType::Dependency,       WidgetBase::wt_Class,      WidgetBase::wt_Enum,        true,   true,   true,   false, Any  },
    { All, Uml::AssociationType::Dependency,       WidgetBase::wt_Interface,  WidgetBase::wt_Enum,        true,   true,   true,   false, Any  },
    { All, Uml::AssociationType::Dependency,       WidgetBase::wt_Component,  WidgetBase::wt_Component,   true,   true,   true,   true,  Any  },
    { All, Uml::AssociationType::Dependency,       WidgetBase::wt_Component,  WidgetBase::wt_Interface,   true,   true,   true,   false, Any  },
    { All, Uml::AssociationType::Dependency,       WidgetBase::wt_Component,  WidgetBase::wt_Artifact,    true,   false,  true,   false, Any  },
    { All, Uml::AssociationType::Dependency,       WidgetBase::wt_Component,  WidgetBase::wt_Package,     true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::Dependency,       WidgetBase::wt_Port,       WidgetBase::wt_Interface,   true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::Dependency,       WidgetBase::wt_Package,    WidgetBase::wt_Artifact,    true,   false,  true,   false, Any  },
    { All, Uml::AssociationType::Dependency,       WidgetBase::wt_Node,       WidgetBase::wt_Node,        true,   false,  false,  false, Any  },
    { All, Uml::AssociationType::Realization,      WidgetBase::wt_Class,      WidgetBase::wt_Interface,   false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Realization,      WidgetBase::wt_Interface,  WidgetBase::wt_Package,     false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Realization,      WidgetBase::wt_Interface,  WidgetBase::wt_Interface,   false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Realization,      WidgetBase::wt_Component,  WidgetBase::wt_Interface,   false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Realization,      WidgetBase::wt_Package,    WidgetBase::wt_Interface,   false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Composition,      WidgetBase::wt_Class,      WidgetBase::wt_Class,       true,   true,   false,  true,  Any  },
    { All, Uml::AssociationType::Composition,      WidgetBase::wt_Interface,  WidgetBase::wt_Interface,   true,   true,   false,  false, Any  },
    { All, Uml::AssociationType::Composition,      WidgetBase::wt_Class,      WidgetBase::wt_Interface,   true,   true,   false,  false, Any  },
    { All, Uml::AssociationType::Composition,      WidgetBase::wt_Class,      WidgetBase::wt_Enum,        true,   true,   false,  false, Any  },
    { All, Uml::AssociationType::Composition,      WidgetBase::wt_Class,      WidgetBase::wt_Datatype,    false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Composition,      WidgetBase::wt_Class,      WidgetBase::wt_Class,       false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Containment,      WidgetBase::wt_Package,    WidgetBase::wt_Class,       false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Containment,      WidgetBase::wt_Package,    WidgetBase::wt_Interface,   false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Containment,      WidgetBase::wt_Package,    WidgetBase::wt_Enum,        false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Containment,      WidgetBase::wt_Package,    WidgetBase::wt_Datatype,    false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Containment,      WidgetBase::wt_Package,    WidgetBase::wt_Package,     false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Containment,      WidgetBase::wt_Package,    WidgetBase::wt_Component,   false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Containment,      WidgetBase::wt_Class,      WidgetBase::wt_Class,       false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Containment,      WidgetBase::wt_Class,      WidgetBase::wt_Interface,   false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Containment,      WidgetBase::wt_Class,      WidgetBase::wt_Enum,        false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Containment,      WidgetBase::wt_Class,      WidgetBase::wt_Datatype,    false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Containment,      WidgetBase::wt_Interface,  WidgetBase::wt_Class,       false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Containment,      WidgetBase::wt_Interface,  WidgetBase::wt_Interface,   false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Containment,      WidgetBase::wt_Interface,  WidgetBase::wt_Enum,        false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Containment,      WidgetBase::wt_Interface,  WidgetBase::wt_Datatype,    false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Containment,      WidgetBase::wt_Component,  WidgetBase::wt_Component,   false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Containment,      WidgetBase::wt_Component,  WidgetBase::wt_Artifact,    false,  false,  false,  false, Any  },
    { All, Uml::AssociationType::Coll_Mesg_Sync,   WidgetBase::wt_Object,     WidgetBase::wt_Object,      true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Coll_Mesg_Sync,   WidgetBase::wt_Instance,   WidgetBase::wt_Instance,    true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Coll_Mesg_Sync,   WidgetBase::wt_Object,     WidgetBase::wt_Instance,    true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Coll_Mesg_Sync,   WidgetBase::wt_Instance,   WidgetBase::wt_Object,      true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Coll_Mesg_Async,  WidgetBase::wt_Object,     WidgetBase::wt_Object,      true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Coll_Mesg_Async,  WidgetBase::wt_Instance,   WidgetBase::wt_Instance,    true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Coll_Mesg_Async,  WidgetBase::wt_Object,     WidgetBase::wt_Instance,    true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Coll_Mesg_Async,  WidgetBase::wt_Instance,   WidgetBase::wt_Object,      true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::State,            WidgetBase::wt_State,      WidgetBase::wt_State,       true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::State,            WidgetBase::wt_ForkJoin,   WidgetBase::wt_State,       true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::State,            WidgetBase::wt_State,      WidgetBase::wt_ForkJoin,    true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Activity,         WidgetBase::wt_Signal,     WidgetBase::wt_Activity,    true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Activity,         WidgetBase::wt_Activity,   WidgetBase::wt_Signal,      true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Activity,         WidgetBase::wt_ObjectNode, WidgetBase::wt_Activity,    true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Activity,         WidgetBase::wt_Activity,   WidgetBase::wt_ObjectNode,  true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Activity,         WidgetBase::wt_Activity,   WidgetBase::wt_Activity,    true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Activity,         WidgetBase::wt_ForkJoin,   WidgetBase::wt_Activity,    true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Activity,         WidgetBase::wt_Activity,   WidgetBase::wt_ForkJoin,    true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Activity,         WidgetBase::wt_Signal,     WidgetBase::wt_ForkJoin,    true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Activity,         WidgetBase::wt_ForkJoin,   WidgetBase::wt_Signal,      true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Activity,         WidgetBase::wt_ForkJoin,   WidgetBase::wt_ObjectNode,  true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Activity,         WidgetBase::wt_ObjectNode, WidgetBase::wt_ForkJoin,    true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Activity,         WidgetBase::wt_Pin,        WidgetBase::wt_Activity,    true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Activity,         WidgetBase::wt_Pin,        WidgetBase::wt_Pin,         true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Activity,         WidgetBase::wt_Activity,   WidgetBase::wt_Pin,         true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Activity,         WidgetBase::wt_Pin,        WidgetBase::wt_ForkJoin,    true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Activity,         WidgetBase::wt_ForkJoin,   WidgetBase::wt_Pin,         true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Anchor,           WidgetBase::wt_Activity,   WidgetBase::wt_Note,        false,  false,  true,   false, Any  },
    { All, Uml::AssociationType::Anchor,           WidgetBase::wt_Actor,      WidgetBase::wt_Note,        false,  false,  true,   false, Any  },
    { All, Uml::AssociationType::Anchor,           WidgetBase::wt_Artifact,   WidgetBase::wt_Note,        false,  false,  true,   false, Any  },
    { All, Uml::AssociationType::Anchor,           WidgetBase::wt_Class,      WidgetBase::wt_Note,        false,  false,  true,   false, Any  },
    { All, Uml::AssociationType::Anchor,           WidgetBase::wt_Component,  WidgetBase::wt_Note,        false,  false,  true,   false, Any  },
    { All, Uml::AssociationType::Anchor,           WidgetBase::wt_Datatype,   WidgetBase::wt_Note,        false,  false,  true,   false, Any  },
    { All, Uml::AssociationType::Anchor,           WidgetBase::wt_Entity,     WidgetBase::wt_Note,        false,  false,  true,   false, Any  },
    { All, Uml::AssociationType::Anchor,           WidgetBase::wt_Enum,       WidgetBase::wt_Note,        false,  false,  true,   false, Any  },
    { All, Uml::AssociationType::Anchor,           WidgetBase::wt_Interface,  WidgetBase::wt_Note,        false,  false,  true,   false, Any  },
    { All, Uml::AssociationType::Anchor,           WidgetBase::wt_Message,    WidgetBase::wt_Note,        false,  false,  true,   false, Any  },
    { All, Uml::AssociationType::Anchor,           WidgetBase::wt_Object,     WidgetBase::wt_Note,        false,  false,  true,   false, Any  },
    { All, Uml::AssociationType::Anchor,           WidgetBase::wt_Package,    WidgetBase::wt_Note,        false,  false,  true,   false, Any  },
    { All, Uml::AssociationType::Anchor,           WidgetBase::wt_State,      WidgetBase::wt_Note,        false,  false,  true,   false, Any  },
    { All, Uml::AssociationType::Anchor,           WidgetBase::wt_UseCase,    WidgetBase::wt_Note,        false,  false,  true,   false, Any  },
    { All, Uml::AssociationType::Relationship,     WidgetBase::wt_Entity,     WidgetBase::wt_Entity,      true,   true,   true,   true,  Any  },
    { All, Uml::AssociationType::Exception,        WidgetBase::wt_Activity,   WidgetBase::wt_Activity,    true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Exception,        WidgetBase::wt_Activity,   WidgetBase::wt_Signal,      true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Exception,        WidgetBase::wt_Signal,     WidgetBase::wt_Activity,    true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Exception,        WidgetBase::wt_Signal,     WidgetBase::wt_Signal,      true,   false,  true,   true,  Any  },
    { All, Uml::AssociationType::Category2Parent,  WidgetBase::wt_Category,   WidgetBase::wt_Entity,      false,  false,  true,   false, Any  },
    { All, Uml::AssociationType::Child2Category,   WidgetBase::wt_Entity,     WidgetBase::wt_Category,    false,  false,  true,   false, Any  }
};

int AssocRules::m_nNumRules = sizeof(m_AssocRules) / sizeof(AssocRules::Assoc_Rule);

bool AssocRules::Assoc_Rule::isValid() const
{
    bool isValidLevel = (level == Any) ||
            (level == UML1 && !Settings::optionState().generalState.uml2) ||
            (level == UML2 && Settings::optionState().generalState.uml2);
    if (language == All)
        return isValidLevel;
    else if (language == Java && UMLApp::app()->activeLanguage() == Uml::ProgrammingLanguage::Java)
        return isValidLevel;
    else
        return false;
}
