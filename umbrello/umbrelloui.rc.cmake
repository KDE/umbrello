<!DOCTYPE kpartgui>
<kpartgui name="umbrello" version="12">
<MenuBar>
  <Menu name="file"><text>&amp;File</text>
      <Menu name="file_export"><text>&amp;Export model</text>
          <Action name="file_export_docbook"/>
          <Action name="file_export_xhtml"/>
      </Menu>
      <Action name="view_export_images"/>
  </Menu>
  <Menu name="edit"><text>&amp;Edit</text>
    <Action name="delete_selected"/>
  </Menu>
  <Menu name="views"><text>&amp;Diagram</text>
    <Menu name="new_view"><text>&amp;New</text>
      <Action name="new_class_diagram"/>
      @ACTION_NEW_OBJECT_DIAGRAM@
      <Action name="new_sequence_diagram"/>
      <Action name="new_collaboration_diagram"/>
      <Action name="new_use_case_diagram"/>
      <Action name="new_state_diagram"/>
      <Action name="new_activity_diagram"/>
      <Action name="new_component_diagram"/>
      <Action name="new_deployment_diagram"/>
      <Action name="new_entityrelationship_diagram"/>
    </Menu>
    <Action name="view_clear_diagram"/>
    <Action name="view_delete"/>
    <Action name="view_export_image"/>
    <Separator/>
    <Menu name="show_view"><text>Sh&amp;ow</text>
      <ActionList name="view_list" />
    </Menu>
    <Menu name="zoom_menu"><text>&amp;Zoom</text>
    </Menu>
    <Menu name="align_menu"><text>&amp;Align</text>
      <Action name="align_left" />
      <Action name="align_right" />
      <Action name="align_top" />
      <Action name="align_bottom" />
      <Action name="align_vertical_middle" />
      <Action name="align_horizontal_middle" />
      <Action name="align_vertical_distribute" />
      <Action name="align_horizontal_distribute" />
    </Menu>
    <Separator/>
    <Action name="view_snap_to_grid"/>
    <Action name="view_show_grid"/>
    <Action name="view_properties"/>
  </Menu>

  <Menu name="code"><text>&amp;Code</text>
    <Action name="import_class"/>
    <Action name="import_project"/>
    <Action name="importing_wizard"/>
    <Action name="generation_wizard"/>
    <Action name="generate_all"/>
    <Menu name="active_lang_menu"><text>Active &amp;Language</text>
      <Action name="setLang_actionscript"/>
      <Action name="setLang_ada"/>
      <Action name="setLang_cpp"/>
      <Action name="setLang_csharp"/>
      <Action name="setLang_d"/>
      <Action name="setLang_idl"/>
      <Action name="setLang_java"/>
      <Action name="setLang_javascript"/>
      <Action name="setLang_mysql"/>
      <Action name="setLang_pascal"/>
      <Action name="setLang_perl"/>
      <Action name="setLang_php"/>
      <Action name="setLang_php5"/>
      <Action name="setLang_postgresql"/>
      <Action name="setLang_python"/>
      <Action name="setLang_ruby"/>
      <Action name="setLang_tcl"/>
      <Action name="setLang_vala"/>
      <Action name="setLang_xmlschema"/>
      <Action name="setLang_none"/>
    </Menu>
    <Separator/>
    <Action name="create_default_datatypes"/>
    <Action name="class_wizard"/>
  </Menu>
  <Menu name="settings"><text>&amp;Settings</text>
    <DefineGroup name="view_show" append="view_show"/>
    <Action name="view_show_tree"/>
    <Action name="view_show_doc"/>
    <Action name="view_show_undo"/>
    <Action name="view_show_bird"/>
    <Action name="view_show_stereotypes"/>
    <Action name="view_show_diagrams"/>
    <Action name="view_show_objects"/>
    <Action name="view_show_welcome"/>
  </Menu>
</MenuBar>
<ToolBar name="mainToolBar" fullWidth="true" newline="true">
</ToolBar>
</kpartgui>
