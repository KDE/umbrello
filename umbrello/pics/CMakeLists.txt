
add_subdirectory(global)

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
    arrow
    artifact
    association
    box
    branch
    category
    category2parent
    check_constraint
    child2category
    choice-rhomb
    choice-round
    class
    class-or-package
    combined_fragment
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
    #unused
    #diag_activity
    #diagbase
    #diag_class
    #diag_collaboration
    #diag_component
    #diag_deployment
    #diag_entityrelationship
    #diag_sequence
    #diag_state
    #diag_usecase
    distribute-horizontal
    distribute-vertical
    end_state
    entity
    enum
    foreignkey_constraint
    #generalise
    #initial
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

set(CURSOR_ICONS
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

if(BUILD_ICONS)
    generate_icons("${ICONS}" 22)
    # for visual inspecting
    #generate_icons("${ICONS}" 128)
endif()

foreach(i ${ICONS})
    install(FILES ${i}.png DESTINATION ${DATA_INSTALL_DIR}/umbrello/pics)
endforeach()

if(BUILD_CURSOR_ICONS)
    generate_cursor_icons("${CURSOR_ICONS}" 32)
endif()


foreach(i ${CURSOR_ICONS})
    install(FILES cursor-${i}.png DESTINATION ${DATA_INSTALL_DIR}/umbrello/pics)
endforeach()

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
