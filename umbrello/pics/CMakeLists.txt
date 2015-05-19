
add_subdirectory(global)

install(FILES actor.png  aggregation.png
        align-horizontal-left.png  align-horizontal-right.png
        align-vertical-top.png  align-vertical-bottom.png
        align-vertical-center.png  align-horizontal-center.png
        distribute-vertical.png  distribute-horizontal.png
        anchor.png  andline.png  arrow.png  association.png
        choice-rhomb.png  choice-round.png  combined_fragment.png
        component.png  region.png  composition.png  containment.png
        CVglobal_meth.png  CVglobal_var.png  CVimplementation_meth.png
        CVimplementation_signal.png  CVimplementation_slot.png  CVimplementation_var.png
        CVnamespace.png  CVprivate_meth.png  CVprivate_signal.png
        CVprivate_slot.png  CVprivate_var.png  CVprotected_meth.png
        CVprotected_signal.png  CVprotected_slot.png  CVprotected_var.png
        CVpublic_meth.png  CVpublic_signal.png  CVpublic_slot.png
        CVpublic_var.png  CVstruct.png
        deep-history.png  shallow-history.png
        dependency.png  final_activity.png  end_state.png  generalisation.png
        initial_state.png  interface.png  join.png  junction.png  datatype.png  enum.png
        umbr-coll-message-synchronous.png  umbr-coll-message-asynchronous.png
        umbr-message-synchronous.png  umbr-message-asynchronous.png
        note.png pin.png object_node.png object.png
        startlogo.png  template.png  text.png  refactor.png
        class.png class-or-package.png uniassociation.png  usecase.png
        category.png child2category.png category2parent.png branch.png
        activity-fork.png  state-fork.png  package.png  artifact.png
        box.png  node.png  port.png  entity.png  relationship.png
        subsystem.png foreignkey_constraint.png unique_constraint.png
        primarykey_constraint.png check_constraint.png
        cursor-actor.png  umbr-message-found.png  accept_signal.png
        accept_time_event.png  send_signal.png  cursor-andline.png
        cursor-choice-rhomb.png  cursor-choice-round.png  cursor-combined_fragment.png
        cursor-deep-history.png  cursor-join.png  cursor-junction.png
        cursor-category.png cursor-child2category.png cursor-category2parent.png
        cursor-shallow-history.png  cursor-state-fork.png  cursor-usecase.png
        cursor-initial_state.png  cursor-package.png  cursor-aggregation.png
        cursor-component.png  cursor-containment.png  cursor-interface.png
        cursor-datatype.png  cursor-enum.png  cursor-text.png
        cursor-anchor.png cursor-pin.png cursor-composition.png
        cursor-umbr-coll-message-synchronous.png  cursor-umbr-coll-message-asynchronous.png
        cursor-umbr-message-synchronous.png  cursor-umbr-message-asynchronous.png
        cursor-class.png  cursor-artifact.png cursor-dependency.png  cursor-uniassociation.png
        cursor-association.png  cursor-end_state.png  cursor-final_activity.png
        cursor-node.png  cursor-box.png  cursor-activity-fork.png
        cursor-note.png  cursor-branch.png  cursor-generalisation.png
        cursor-object.png  cursor-entity.png  cursor-relationship.png
        precondition.png  cursor-precondition.png  cursor-send_signal.png
        cursor-accept_signal.png  cursor-accept_time_event.png  cursor-region.png
        cursor-umbr-message-found.png  umbr-message-lost.png  cursor-umbr-message-lost.png
        end_of_life.png  cursor-end_of_life.png  exception.png
        cursor-exception.png cursor-object_node.png
        cursor-PrePostCondition.png PrePostCondition.png
    DESTINATION ${DATA_INSTALL_DIR}/umbrello/pics
)
if(NOT BUILD_KF5)
    kde4_install_icons(${DATA_INSTALL_DIR}/umbrello/icons)
else()
    ecm_install_icons(
        ICONS   hi16-actions-umbrello_diagram_activity.png
                hi16-actions-umbrello_diagram_class.png
                hi16-actions-umbrello_diagram_collaboration.png
                hi16-actions-umbrello_diagram_component.png
                hi16-actions-umbrello_diagram_deployment.png
                hi16-actions-umbrello_diagram_sequence.png
                hi16-actions-umbrello_diagram_state.png
                hi16-actions-umbrello_diagram_usecase.png
                hi22-actions-umbrello_diagram_activity.png
                hi22-actions-umbrello_diagram_class.png
                hi22-actions-umbrello_diagram_collaboration.png
                hi22-actions-umbrello_diagram_component.png
                hi22-actions-umbrello_diagram_deployment.png
                hi22-actions-umbrello_diagram_entityrelationship.png
                hi22-actions-umbrello_diagram_sequence.png
                hi22-actions-umbrello_diagram_state.png
                hi22-actions-umbrello_diagram_usecase.png
        THEME hicolor
        DESTINATION ${ICON_INSTALL_DIR}
    )
endif()
