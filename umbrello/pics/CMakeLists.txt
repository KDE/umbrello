
add_subdirectory(global)

if(BUILD_ICONS)
    set(ICONS
        accept_signal
        accept_time_event
        activity-fork
        actor
        aggregation
        align-horizontal-center
        align-horizontal-left
        align-horizontal-right
        align-vertical-bottom
        align-vertical-center
        align-vertical-top
        anchor
        andline
        artifact
        association
        asynchron
        box
        branch
        category2parent
        category
        check_constraint
        child2category
        choice-rhomb
        choice-round
        class-or-package
        class
        component
        composition
        containment
        CVglobal_meth
        CVglobal_var
        CVimplementation_meth
        CVimplementation_signal
        CVimplementation_slot
        CVimplementation_var
        CVnamespace
        CVprivate_meth
        CVprivate_signal
        CVprivate_slot
        CVprivate_var
        CVprotected_meth
        CVprotected_signal
        CVprotected_slot
        CVprotected_var
        CVpublic_meth
        CVpublic_signal
        CVpublic_slot
        CVpublic_var
        CVstruct
        datatype
        deep-history
        dependency
        diag_activity
        diagbase
        diag_class
        diag_collaboration
        diag_component
        diag_deployment
        diag_entityrelationship
        diag_sequence
        diag_state
        diag_usecase
        distribute-horizontal
        distribute-vertical
        end_state
        entity
        enum
        foreignkey_constraint
        generalise
        initial
        interface
        join
        junction
        node
        note
        object
        package
        port
        primarykey_constraint
        realization
        refactor
        relationship
        send_signal
        shallow-history
        state-fork
        subsystem
        template
        text
        umbr-coll-message-asynchronous
        umbr-coll-message-synchronous
        umbr-message-asynchronous
        umbr-message-synchronous
        uniassociation
        unique_constraint
        usecase
    )
    generate_icons("${ICONS}" 22)
    # for visual inspecting
    #generate_icons("${ICONS}" 128)
endif()

if(BUILD_CURSOR_ICONS)
    set(CURSOR_ICONS
        send_signal
        accept_signal
        accept_time_event
        activity-fork
        actor
        aggregation
        anchor
        andline
        artifact
        association
        box
        branch
        category2parent
        category
        child2category
        choice-rhomb
        choice-round
        class
        combined_fragment
        component
        composition
        containment
        datatype
        deep-history
        dependency
        end_of_life
        end_state
        entity
        enum
        exception
        final_activity
        generalisation
        initial_state
        interface
        join
        junction
        node
        note
        object_node
        object
        package
        pin
        precondition
        PrePostCondition
        region
        relationship
        send_signal
        shallow-history
        state-fork
        text
        umbr-coll-message-asynchronous
        umbr-coll-message-synchronous
        umbr-message-asynchronous
        umbr-message-found
        umbr-message-lost
        umbr-message-synchronous
        uniassociation
        usecase
    )
    generate_cursor_icons("${CURSOR_ICONS}" 32)
endif()


install(FILES actor  aggregation
        align-horizontal-left  align-horizontal-right
        align-vertical-top  align-vertical-bottom
        align-vertical-center  align-horizontal-center
        distribute-vertical  distribute-horizontal
        anchor  andline  arrow  association
        choice-rhomb  choice-round  combined_fragment
        component  region  composition  containment
        CVglobal_meth  CVglobal_var  CVimplementation_meth
        CVimplementation_signal  CVimplementation_slot  CVimplementation_var
        CVnamespace  CVprivate_meth  CVprivate_signal
        CVprivate_slot  CVprivate_var  CVprotected_meth
        CVprotected_signal  CVprotected_slot  CVprotected_var
        CVpublic_meth  CVpublic_signal  CVpublic_slot
        CVpublic_var  CVstruct
        deep-history  shallow-history
        dependency  final_activity  end_state  generalisation
        initial_state  interface  join  junction  datatype  enum
        umbr-coll-message-synchronous  umbr-coll-message-asynchronous
        umbr-message-synchronous  umbr-message-asynchronous
        note pin object_node object
        startlogo  template  text  refactor
        class class-or-package uniassociation  usecase
        category child2category category2parent branch
        activity-fork  state-fork  package  artifact
        box  node  port  entity  relationship
        subsystem foreignkey_constraint unique_constraint
        primarykey_constraint check_constraint
        cursor-actor  umbr-message-found  accept_signal
        accept_time_event  send_signal  cursor-andline
        cursor-choice-rhomb  cursor-choice-round  cursor-combined_fragment
        cursor-deep-history  cursor-join  cursor-junction
        cursor-category cursor-child2category cursor-category2parent
        cursor-shallow-history  cursor-state-fork  cursor-usecase
        cursor-initial_state  cursor-package  cursor-aggregation
        cursor-component  cursor-containment  cursor-interface
        cursor-datatype  cursor-enum  cursor-text
        cursor-anchor cursor-pin cursor-composition
        cursor-umbr-coll-message-synchronous  cursor-umbr-coll-message-asynchronous
        cursor-umbr-message-synchronous  cursor-umbr-message-asynchronous
        cursor-class  cursor-artifact cursor-dependency  cursor-uniassociation
        cursor-association  cursor-end_state  cursor-final_activity
        cursor-node  cursor-box  cursor-activity-fork
        cursor-note  cursor-branch  cursor-generalisation
        cursor-object  cursor-entity  cursor-relationship
        precondition  cursor-precondition  cursor-send_signal
        cursor-accept_signal  cursor-accept_time_event  cursor-region
        cursor-umbr-message-found  umbr-message-lost  cursor-umbr-message-lost
        end_of_life  cursor-end_of_life  exception
        cursor-exception cursor-object_node
        cursor-PrePostCondition PrePostCondition
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
