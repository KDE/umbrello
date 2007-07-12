/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef LISTPOPUPMENU_H
#define LISTPOPUPMENU_H

#include <kpopupmenu.h>
#include "umlnamespace.h"

class UMLView;
class UMLWidget;
class ClassifierWidget;

/**
 * A popup menu that depending on what type it is set to will
 * display a different menu.
 *
 * @short Displays a popup menu.
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class ListPopupMenu : public KPopupMenu {
public:

    /// This type hosts all possible menu types.
    enum Menu_Type
    {
        mt_Model,
        mt_Logical_View,
        mt_UseCase_View,
        mt_Component_View,
        mt_Deployment_View,
        mt_EntityRelationship_Model,
        mt_UseCase_Diagram,
        mt_Sequence_Diagram,
        mt_Class_Diagram,
        mt_Collaboration_Diagram,
        mt_State_Diagram,
        mt_Activity_Diagram,
        mt_Component_Diagram,
        mt_Deployment_Diagram,
        mt_EntityRelationship_Diagram,
        mt_On_UseCase_Diagram,
        mt_On_Sequence_Diagram,
        mt_On_Class_Diagram,
        mt_On_Collaboration_Diagram,
        mt_On_State_Diagram,
        mt_On_Activity_Diagram,
        mt_On_Component_Diagram,
        mt_On_Deployment_Diagram,
        mt_On_EntityRelationship_Diagram,
        mt_Logical_Folder,
        mt_UseCase_Folder,
        mt_Component_Folder,
        mt_Deployment_Folder,
        mt_EntityRelationship_Folder,
        mt_Class,
        mt_Package,
        mt_Subsystem,
        mt_Component,
        mt_Node,
        mt_Artifact,
        mt_Interface,
        mt_Enum,
        mt_Entity,
        mt_Datatype,
        mt_Actor,
        mt_UseCase,
        mt_Attribute,
        mt_EntityAttribute,
        mt_EnumLiteral,
        mt_Object,
        mt_Initial_State,
        mt_End_State,
        mt_State,
        mt_Activity,
        mt_Initial_Activity,
        mt_End_Activity,
        mt_Operation,
        mt_Template,
        mt_New_Parameter,
        mt_New_Operation,
        mt_New_Attribute,
        mt_New_Template,
        mt_New_EnumLiteral,
        mt_New_EntityAttribute,
        mt_Parameter_Selected,
        mt_Operation_Selected,
        mt_Attribute_Selected,
        mt_Template_Selected,
        mt_EnumLiteral_Selected,
        mt_EntityAttribute_Selected,
        mt_Association_Selected,          // Association without role names
        mt_Show_Attributes,
        mt_Show_Attributes_Selection,     //SHOWATTS, multiple items
        mt_Show_Operations,
        mt_Show_Operations_Selection,     //SHOWOPS, multiple items
        mt_Show_Packages,
        mt_Show_Packages_Selection,       //SHOWPACKAGE, multiple items
        mt_Show_Stereotypes,
        mt_Show_Stereotypes_Selection,    //SHOWSTEREOTYPE, multiple items
        mt_Visibility,
        mt_Visibility_Selection,               //SCOPE, multiple items
        mt_DrawAsCircle,
        mt_DrawAsCircle_Selection,        //DRAWASCIRCLE, multiple items
        mt_ChangeToClass,
        mt_ChangeToClass_Selection,
        mt_ChangeToInterface,
        mt_ChangeToInterface_Selection,
        mt_Rename_Object,
        mt_Select_Operation,
        mt_Anchor,
        mt_Properties,
        mt_Rename,
        mt_Delete,
        mt_Export_Image,
        mt_Import_Classes,
        mt_Sequence_Number,
        mt_Cut,
        mt_Copy,
        mt_Paste,
        mt_Clear,
        mt_Redo,
        mt_Undo,
        mt_Link_Docs,
        mt_Show_Operation_Signature,
        mt_Show_Operation_Signature_Selection, //SHOWOPSIG, multiple items
        mt_Show_Attribute_Signature,
        mt_Show_Attribute_Signature_Selection, //SHOWATTSIG, multiple items
        mt_Message_Text,
        mt_Collaboration_Message,
        mt_FloatText,
        mt_MultiA,
        mt_MultiB,
        mt_Name,                          //Association name
        mt_FullAssociation,                 // Association with role names
        mt_AttributeAssociation,   // Rendering of an attribute as an association
        mt_RoleNameA,
        mt_RoleNameB,
        mt_Delete_Selection,
        mt_Reset_Label_Positions,
        mt_Line_Color,
        mt_Line_Color_Selection,          //LINECOLOR, multiple items
        mt_Fill_Color,
        mt_Fill_Color_Selection,          //FILLCOLOR, multiple items
        mt_Use_Fill_Color,
        mt_Default_Properties,
        mt_Rename_MultiA,
        mt_Rename_MultiB,
        mt_Rename_Name,
        mt_Rename_RoleAName,
        mt_Rename_RoleBName,
        mt_Change_Font,
        mt_Change_Font_Selection,
        mt_SnapToGrid,
        mt_ShowSnapGrid,
        mt_Activity_Selected,
        mt_New_Activity,
        mt_Up,
        mt_Down,
        mt_Branch,
        mt_Flip,

        mt_Expand_All,                     //Expand all items in the list
        mt_Collapse_All,                   //Collapse all items in the list

        mt_Refactoring,
        mt_ViewCode, // view code document contents
        mt_Clone, // Create a deep copy of the object.
        mt_Show_Public_Only, // (not currently used)
        mt_Show_Public_Only_Selection,  // Show public operations/attributes only.
        mt_Externalize_Folder,  // Mark folder for saving as separate submodel
        mt_Internalize_Folder,  // Reintegrate separate submodel into main model

        mt_Undefined  =  - 1
    };

    /**
     * Constructs the popup menu for a diagram
     *
     * @param parent    The parent to ListPopupMenu.
     * @param type      The type of menu to display.
     * @param view      The UMLView in which this ListPopupMenu is going to be displayed
     */
    explicit ListPopupMenu(QWidget* parent, Menu_Type type = mt_Undefined, UMLView* view = 0);

    /**
     * Constructs the popup menu for a list view item.
     *
     * @param parent    The parent to ListPopupMenu.
     * @param type              The type of menu to display.
     */
    ListPopupMenu(QWidget* parent, Uml::ListView_Type type);

    /**
     * Constructs the popup menu for a canvas widget.
     *
     * @param parent    The parent to ListPopupMenu.
     * @param object    The UMLWidget to represent a menu for.
     * @param multi             True if multiple items are selected.
     * @param unique    True if multiple selected items all have
     *                  the same type (e.g. Class, Interface)
     */
    ListPopupMenu(QWidget* parent, UMLWidget* object, bool multi = false, bool unique = false);

    /**
     * Standard deconstructor.
     */
    ~ListPopupMenu();

    /**
     * Utility: Convert a Menu_Type value to an Object_Type value.
     */
    static Uml::Object_Type convert_MT_OT(Menu_Type mt);

    /**
     * Utility: Convert a Menu_Type value to a Diagram_Type value.
     */
    static Uml::Diagram_Type convert_MT_DT(Menu_Type mt);

private:
    /**
     * Basic initialization - common to all constructors.
     */
    void init();

    /**
     * Shortcut for inserting a "File->New" choice.
     */
    void insertFileNew();

    /**
     * Shortcut for the most frequently used insertItem() calls.
     *
     * @param m The Menu_Type for which to insert a menu item.
     */
    void insertStdItem(Menu_Type m);

    /**
     * Shortcut for the most frequently used insertStdItem() calls.
     *
     * @param insertLeadingSeparator  Set this true if the group shall
     *                            start with a separator.
     * @param type      The Widget_Type for which to insert the menu items.
     *          If no argument is supplied then a Rename item will be
     *          included.
     */
    void insertStdItems(bool insertLeadingSeparator = true,
                        Uml::Widget_Type type = Uml::wt_UMLWidget);

    /**
     * Shortcut for inserting standard model items (Class, Interface,
     * Datatype, Enum, Package) as well as diagram choices.
     *
     * @param folderAndDiagrams Set this true if folders and diagram
     *                          types shall be included as choices.
     */
    void insertContainerItems(bool folderAndDiagrams);

    /**
     * Inserts a menu item for an association related text
     * (such as name, role, multiplicity etc.)
     *
     * @param label             The menu text.
     * @param mt                The menu type.
     */
    void insertAssocItem(const QString &label, Menu_Type mt);

    /**
     * Inserts a menu item for externalization/de-externalization
     * of a folder.
     */
    void insertSubmodelAction();

    /**
     * Creates a popup menu for a multiple selection of class and
     * interface widgets.
     */
    void makeMultiClassifierPopup(ClassifierWidget *c);

    /**
     * Creates a popup menu for a single class or interface widgets.
     */
    void makeClassifierPopup(ClassifierWidget *c);

    /**
     * Shortcut for commonly used menu initializations.
     *
     * @param type      The Menu_Type for which to set up the menu.
     * @param view      The UMLView parent of the menu.
     */
    void setupMenu(Menu_Type type, UMLView * view = 0);

    enum PixMap_Type {
        pm_Class,
        pm_Package,
        pm_Interface,
        pm_Datatype,
        pm_Enum,
        pm_Actor,
        pm_Usecase,
        pm_InitialState,
        pm_EndState,
        pm_Branch,
        pm_Object,
        pm_Component,
        pm_Node,
        pm_Artifact,
        pm_Text,
        pm_Entity,
        pm_Subsystem,
        pm_NUMBER_OF_PIXMAPS
    };
    QPixmap m_pixmap[pm_NUMBER_OF_PIXMAPS];
    KPopupMenu * m_pInsert, * m_pShow, * m_pColor;
    void setupColor(bool fc);
    void setupColorSelection(bool fc);
    void setupDiagramMenu(UMLView* view);
};

#endif
