<?xml version="1.0"?>
<!DOCTYPE article PUBLIC "-//OASIS//DTD Simplified DocBook XML V4.1.2.5//EN"
          "http://www.oasis-open.org/docbook/xml/simple/4.1.2.5/sdocbook.dtd" 
          [ <!ENTITY % common SYSTEM "common.ent" > %common;] >

<!--
    Title: umbrello-xmi-to-html.xsl
    Purpose: An XSL stylesheet for converting Umbrello 1.4 XMI to HTML.
             Based on xmi-to-html.xsl from Objects by Design.

    Copyright (C) 1999-2001, Objects by Design, Inc. All Rights Reserved.
    Copyright (C) 2005, Oliver M. Kellogg <okellogg@users.sourceforge.net>
    Copyright (C) 2006, Gaël de Chalendar (Kleag) <kleag@free.fr>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation. A copy of the license may be found at
    http://www.gnu.org/licenses/gpl.html

    Version:  June, 16 2006

    xmlns="http://www.w3.org/1999/xhtml"
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0"
                xmlns:UML="http://schema.omg.org/spec/UML/1.3"
                exclude-result-prefixes="UML">
                
<xsl:output method="xml" indent="yes" 
            doctype-system="http://www.oasis-open.org/docbook/xml/simple/4.1.2.5/sdocbook.dtd"
            doctype-public="-//OASIS//DTD Simplified DocBook XML V4.1.2.5//EN" />

<xsl:key
    name="classifier"
    match="//UML:Class"
    use="@xmi.id"/>

<xsl:key
    name="generalization"
    match="//UML:Generalization"
    use="@xmi.id"/>

<xsl:key
    name="abstraction"
    match="//UML:Abstraction"
    use="@xmi.id"/>

<xsl:key
    name="multiplicity"
    match="//UML:Multiplicity"
    use="@xmi.id"/>


<!-- Document Root -->
<xsl:template match="/">
    <article role="specification">
    <xsl:apply-templates select="//UML:Model" mode="title"/>
    
    <!-- Actors -->
    <section id="actors">
      <title>Actors</title>
      <xsl:apply-templates select="//UML:Actor"/>
    </section>
    
    <!-- Use Cases -->
    <section id="usecases">
      <title>Use Cases</title>
      <xsl:apply-templates select="//UML:UseCase"/>
    </section>
    
    <!-- Interfaces -->
    <section id="interfaces">
      <title>Interfaces</title>
      <xsl:apply-templates select="//UML:Interface"/>
    </section>
    
      <!-- Classes -->
      <section id="classes">
        <title>Classes</title>
        <xsl:apply-templates select="//UML:Class"/>
      </section>
      <!-- Diagrams -->
      <section id="diagrams">
        <title>Diagrams</title>
        <xsl:apply-templates select="//diagrams/diagram"/>
      </section>
    </article>

</xsl:template>


<!-- Window Title -->
<xsl:template match="UML:Model" mode="title">
    <title>
      <!-- Name of the model -->
      <xsl:value-of select="@name"/>
    </title>
</xsl:template>


<!-- Actor -->
<xsl:template match="UML:Actor">
  <xsl:variable name="element_name" select="@name"/>
  <xsl:variable name="xmi_id" select="@xmi.id" />
  <xsl:variable name="comment" select="@comment" />
  
  <section>
    <title><xsl:value-of select="$element_name"/></title>
    
    <table frame='all'><title></title>
      <tgroup cols='3' align='left' colsep='1' rowsep='1'>
        <colspec colname='c1'/>
        <colspec colname='c2'/>
        <colspec colname='c3'/>
        <thead>
          <row>
            <entry role="class-title" >Actor</entry>
            <entry role="class-name" namest="c2" nameend="c3" ><xsl:value-of select="$element_name"/></entry>
          </row>
        </thead>
        <tbody>
          <xsl:if test="count($comment) > 0">
            <row>
              <entry role="comment" namest="c1" nameend="c3" ><para><xsl:value-of select="$comment"/></para></entry>
            </row>
          </xsl:if>  
          
          <xsl:call-template name="specifications"/>
          
          <xsl:call-template name="realizations"/>
          
          <xsl:call-template name="supertypes"/>
          
          <xsl:call-template name="subtypes"/>
          
          <xsl:call-template name="associations">
            <xsl:with-param name="source" select="$xmi_id"/>
          </xsl:call-template>
          
        </tbody>
      </tgroup>
    </table>
    
  </section>
</xsl:template>

<!-- Use Case -->
<xsl:template match="UML:UseCase">
  <xsl:variable name="element_name" select="@name"/>
  <xsl:variable name="xmi_id" select="@xmi.id" />
  <xsl:variable name="comment" select="@comment" />
  
  <section>
    <title><xsl:value-of select="$element_name"/></title>
    
    <table frame='all'><title></title>
      <tgroup cols='3' align='left' colsep='1' rowsep='1'>
        <colspec colname='c1'/>
        <colspec colname='c2'/>
        <colspec colname='c3'/>
        <thead>
          <row>
            <entry role="class-title" >Use Case</entry>
            <entry role="class-name" namest="c2" nameend="c3" ><xsl:value-of select="$element_name"/></entry>
          </row>
        </thead>
        <tbody>
          <xsl:if test="count($comment) > 0">
            <row>
              <entry role="comment" namest="c1" nameend="c3" ><para><xsl:value-of select="$comment"/></para></entry>
            </row>
          </xsl:if>  
          
          <xsl:call-template name="specifications"/>
          
          <xsl:call-template name="realizations"/>
          
          <xsl:call-template name="supertypes"/>
          
          <xsl:call-template name="subtypes"/>
          
          <xsl:call-template name="associations">
            <xsl:with-param name="source" select="$xmi_id"/>
          </xsl:call-template>
          
        </tbody>
      </tgroup>
    </table>
    
  </section>
</xsl:template>


<!-- Interface -->
<xsl:template match="UML:Interface">
  <xsl:variable name="element_name" select="@name"/>
  <xsl:variable name="comment" select="@comment" />
  <xsl:variable name="realizations" 
                select="Foundation.Core.ModelElement.supplierDependency/
                Foundation.Core.Abstraction"/>
  <xsl:variable name="generalizations" 
                select="UML:Generalization"/>
  <xsl:variable name="specializations" 
                select="Foundation.Core.GeneralizableElement.specialization/
                Foundation.Core.Generalization"/>
  <xsl:variable name="class_operations" 
                select="UML:Classifier.feature/UML:Operation" />
  <section>
    <title><xsl:value-of select="$element_name"/></title>
    <table frame='all'><title></title>
      <tgroup cols='3' align='left' colsep='1' rowsep='1'>
        <colspec colname='c1'/>
        <colspec colname='c2'/>
        <colspec colname='c3'/>
        <thead>
          <row>
            <entry role="class-title" >Interface</entry>
            <entry role="class-name" namest="c2" nameend="c3" ><xsl:value-of select="$element_name"/></entry>
          </row>
        </thead>
        <tbody>
          <xsl:if test="count($comment) > 0">
            <row>
              <entry role="comment" namest="c1" nameend="c3" ><para><xsl:value-of select="$comment"/></para></entry>
            </row>
          </xsl:if>  
          <xsl:if test="count($class_operations) = 0">
            <row>
              <entry namest="c1" nameend="c3" ><para/></entry>
            </row>
          </xsl:if>
          
          <xsl:if test="count($realizations) = 0 
                  and count($generalizations) = 0
                  and count($specializations) = 0
                  and count($class_operations) = 0">
          </xsl:if>
          <xsl:call-template name="realizations"/>
          <xsl:call-template name="supertypes"/>
          <xsl:call-template name="subtypes"/>
          <xsl:call-template name="operations"/>
        </tbody>
      </tgroup>
    </table>
  
  </section>
</xsl:template>


<!-- Class -->
<xsl:template match="UML:Class">
  <xsl:variable name="element_name" select="@name"/>
  <xsl:variable name="xmi_id" select="@xmi.id" />
  <xsl:variable name="comment" select="@comment" />
  <xsl:variable name="class_attributes" 
                select="UML:Classifier.feature/UML:Attribute" />
  <xsl:variable name="class_operations" 
                select="UML:Classifier.feature/UML:Operation" />
  
  <section>
    <title><xsl:value-of select="$element_name"/></title>
    
  <table frame='all'><title></title>
    <tgroup cols='3' align='left' colsep='1' rowsep='1'>
      <colspec colname='c1'/>
      <colspec colname='c2'/>
      <colspec colname='c3'/>
      <thead>
        <row>
          <entry role="class-title" >Class</entry>
          <entry role="class-name" namest="c2" nameend="c3" ><xsl:value-of select="$element_name"/></entry>
        </row>
      </thead>
      <tbody>
        <xsl:if test="count($comment) > 0">
          <row>
            <entry role="comment" namest="c1" nameend="c3" ><para><xsl:value-of select="$comment"/></para></entry>
          </row>
        </xsl:if>  
        <xsl:if test="count($class_attributes) = 0 
                and count($class_operations) = 0">
          <row>
            <entry namest="c1" nameend="c3" ><para/></entry>
          </row>
        </xsl:if>
        
        <xsl:call-template name="specifications"/>
        
        <xsl:call-template name="realizations"/>
        
        <xsl:call-template name="supertypes"/>
        
        <xsl:call-template name="subtypes"/>
        
        <xsl:call-template name="associations">
          <xsl:with-param name="source" select="$xmi_id"/>
        </xsl:call-template>
        
        <xsl:call-template name="attributes"/>
        
        <xsl:call-template name="operations"/>
      </tbody>
    </tgroup>
  </table>
    
  </section>
</xsl:template>


<xsl:template match="diagrams/diagram">
  <xsl:variable name="xmi_id" select="@xmi.id" />
  <xsl:comment >diagrambegin<xsl:value-of select="position()"/>namebegin<xsl:value-of select="@name"/>nameend</xsl:comment>   
  <xsl:comment >diagram<xsl:value-of select="position()"/>documentationbegin<xsl:value-of select="@documentation"/>documentationend</xsl:comment>   
  
  <section>
    <title><xsl:value-of select="@name"/></title>
    <para>
      <xsl:value-of select="@documentation"/>
    </para>
    <mediaobject>
      <imageobject>
        <imagedata format="PNG">
          <xsl:attribute name="fileref"><xsl:value-of select="@name"/>
            <xsl:text>.png</xsl:text>
          </xsl:attribute>
        </imagedata>
      </imageobject>
    </mediaobject>
  </section>
  <xsl:if test="widgets/actorwidget">
    <section>
      <title>&actor;</title>
      <xsl:for-each select="widgets/actorwidget">
        <xsl:call-template name="actor"/>
      </xsl:for-each>
    </section>
  </xsl:if>
  
  <xsl:if test="widgets/usecasewidget">
    <section>
      <title>&usecase;</title>
      <xsl:for-each select="widgets/usecasewidget">
        <xsl:call-template name="usecase"/>
      </xsl:for-each>
    </section>
  </xsl:if>
  
  <xsl:if test="widgets/classwidget">
    <section>
      <title>&classes;</title>
      <para/>
      <!--xsl:for-each select="widgets/classwidget">
        <xsl:call-template name="class"/>
      </xsl:for-each-->
    </section>
  </xsl:if>
  
  <xsl:comment >diagramend<xsl:value-of select="position()"/></xsl:comment>
</xsl:template>


<!-- The following template is designed to be called for Abstractions with the "realize"
     stereotype, i.e. Realization relationships.  It formats the name of the supplier of the
     Abstraction dependency, i.e. the class or interface specifying the behaviour of the
     client in the dependency.
-->
<!-- Specifications (interface or class) -->
<xsl:template name="specifications">

  <!-- Abstractions identify specifications -->
  <xsl:variable name="specifications" 
        select="Foundation.Core.ModelElement.clientDependency/
                Foundation.Core.Abstraction"/>

  <xsl:if test="count($specifications) > 0">
    <section>
    <title>Specifications:</title>
      <xsl:for-each select="$specifications">
        <!-- get the supplier in the abstraction -->
        <xsl:variable name="abstraction"
              select="key('abstraction', ./@xmi.idref)" />
        <xsl:variable name="target"
              select="$abstraction/
                    Foundation.Core.Dependency.supplier/
                    */@xmi.idref" /> 
        <xsl:call-template name="classify">
            <xsl:with-param name="target" select="$target"/>
        </xsl:call-template>
        
        <xsl:if test="position() != last()">
            <xsl:text>,  </xsl:text>
        </xsl:if>
      </xsl:for-each>
    </section>
  </xsl:if>  
        
</xsl:template>


<!-- The following template is designed to be called for Abstractions with the "realize"
     stereotype, i.e. Realization relationships.  It formats the name of the client of the
     Abstraction dependency, i.e. the class realizing the specification defined by the
     supplier in the dependency.
-->
<!-- Realizations (of interface) -->
<xsl:template name="realizations">

  <!-- Abstractions identify realizations -->
  <xsl:variable name="realizations" 
        select="Foundation.Core.ModelElement.supplierDependency/
                Foundation.Core.Abstraction"/>

  <xsl:if test="count($realizations) > 0">
    <section>
      <title>Realizations:</title>
      <xsl:for-each select="$realizations">
      
        <!-- get the client in the abstraction -->
        <xsl:variable name="abstraction"
              select="key('abstraction', ./@xmi.idref)" />
        <xsl:variable name="target"
              select="$abstraction/
                    Foundation.Core.Dependency.client/
                    */@xmi.idref" /> 
        <xsl:call-template name="classify">
            <xsl:with-param name="target" select="$target"/>
        </xsl:call-template>
        
        <xsl:if test="position() != last()">
            <xsl:text>,  </xsl:text>
        </xsl:if>
      </xsl:for-each>
    </section>
  </xsl:if>  
        
</xsl:template>



<!-- Supertypes (inheritance) -->
<xsl:template name="supertypes">
    
  <!-- Generalizations identify supertypes -->
  <xsl:variable name="generalizations" 
        select="UML:Generalization"/>

  <xsl:if test="count($generalizations) > 0">
    <section>
      <title>Supertypes:</title>
      <xsl:for-each select="$generalizations">
      
        <!-- get the parent in the generalization -->
        <xsl:variable name="generalization"
              select="key('generalization', ./@xmi.idref)" />
        <xsl:variable name="target"
              select="$generalization/
                    Foundation.Core.Generalization.parent/
                    */@xmi.idref" /> 
        <xsl:call-template name="classify">
            <xsl:with-param name="target" select="$target"/>
        </xsl:call-template>
        
        <xsl:if test="position() != last()">
          <xsl:text>,  </xsl:text>
        </xsl:if>
      </xsl:for-each>
    </section>
  </xsl:if>  
</xsl:template>


<!-- Subtypes (inheritance) -->
<xsl:template name="subtypes">
    
  <!-- Specializations identify subtypes -->
  <xsl:variable name="specializations" 
        select="Foundation.Core.GeneralizableElement.specialization/
                Foundation.Core.Generalization"/>

  <xsl:if test="count($specializations) > 0">
    <section>
      <title>Subtypes:</title>
      <xsl:for-each select="$specializations">
    
        <!-- get the child in the generalization -->
        <xsl:variable name="generalization"
              select="key('generalization', ./@xmi.idref)" />
        <xsl:variable name="target"
              select="$generalization/
                    Foundation.Core.Generalization.child/
                    */@xmi.idref" /> 
        <xsl:call-template name="classify">
            <xsl:with-param name="target" select="$target"/>
        </xsl:call-template>
        
        <xsl:if test="position() != last()">
            <xsl:text>,  </xsl:text>
        </xsl:if>
      </xsl:for-each>
    </section>
  </xsl:if>  
</xsl:template>


<!-- Associations -->
<xsl:template name="associations">
  <xsl:param name="source"/>
  
  <xsl:variable name="association_ends" 
        select="//UML:AssociationEnd[@type=$source]" />

  <xsl:if test="count($association_ends) > 0">
    <section>
      <title>Associations</title>
          <para>visibility, type, properties.</para>

      <xsl:for-each select="$association_ends">
        <xsl:for-each select="preceding-sibling::UML:AssociationEnd |
                              following-sibling::UML:AssociationEnd">

          <xsl:call-template name="association_end" />
        
        </xsl:for-each>
      </xsl:for-each>
    </section>
  </xsl:if>
</xsl:template>


<!-- Association End -->
<xsl:template name="association_end">
  <!-- Visibility -->
  <para>
  <!--role="feature-detail"-->
      <xsl:variable name="visibility"
            select="@visibility" />
      <xsl:choose>
          <xsl:when test="string-length($visibility) > 0">
              <xsl:value-of select="$visibility"/>
          </xsl:when>
          <xsl:otherwise>
              <xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text>
          </xsl:otherwise>
      </xsl:choose>        
  </para>
  
  <!-- Type -->
  <para>
  <!--role="feature-detail"-->
      <xsl:variable name="target"
            select="Foundation.Core.AssociationEnd.type/*/@xmi.idref" />

      <xsl:call-template name="classify">
          <xsl:with-param name="target" select="$target"/>
      </xsl:call-template>
  </para>
  
  <!-- Properties -->
  <para>
  <!--role="feature-detail"-->
    <!-- Rolename -->
    <xsl:variable name="rolename" select="@name"/>

      <!--role="property-name"-->
      Rolename: 
    <xsl:choose>
        <xsl:when test="string-length($rolename) > 0">
            <xsl:value-of select="$rolename"/>
        </xsl:when>
        <xsl:otherwise>(none)</xsl:otherwise>
    </xsl:choose>
  </para>
    
    <!-- Multiplicity -->
    <xsl:apply-templates select=".//Foundation.Data_Types.Multiplicity" />

    <!-- Navigable -->
    <xsl:variable name="navigable" 
          select="Foundation.Core.AssociationEnd.isNavigable/@xmi.value"/>
    <xsl:if test="string-length($navigable) > 0">
        
      <para> 
        <!--role="property-name"-->
        Navigable: <xsl:value-of select="$navigable"/>
      </para>
    </xsl:if>
    
    <!-- Ordering -->
    <xsl:variable name="ordering" 
          select="Foundation.Core.AssociationEnd.ordering/@xmi.value"/>
    <xsl:if test="string-length($ordering) > 0">
      <para>
        <!--role="property-name"-->
        Ordering: <xsl:value-of select="$ordering"/>
      </para>
    </xsl:if>
</xsl:template>


<!-- Multiplicity (definition) -->
<xsl:template match="Foundation.Data_Types.Multiplicity[@xmi.id]">
    
  <para>
    <!--role="property-name"-->
    Multiplicity: 
    
    <xsl:variable name="lower" 
         select=".//Foundation.Data_Types.MultiplicityRange.lower"/>
	 
    <xsl:variable name="upper" 
         select=".//Foundation.Data_Types.MultiplicityRange.upper"/>

    <xsl:value-of select="$lower" />
    <xsl:if test="$upper != $lower">
        <xsl:text>..</xsl:text>
        <xsl:value-of select="$upper" />
    </xsl:if>
  </para>
    
</xsl:template>

<!-- Multiplicity (reference) -->
<xsl:template match="Foundation.Data_Types.Multiplicity[@xmi.idref]">
    <xsl:apply-templates select="key('multiplicity', @xmi.idref)" />
</xsl:template>



<!-- Attributes -->
<xsl:template name="attributes">
  <xsl:variable name="class_attributes" 
                select="UML:Classifier.feature/UML:Attribute" />
  <xsl:if test="count($class_attributes) > 0">
    <row>
      <entry role="info-title"  namest="c1" nameend="c3" >Attributes:</entry>
    </row>
    <row>
      <entry role="feature-heading">visibility</entry>
      <entry role="feature-heading">type</entry>
      <entry role="feature-heading">name</entry>
    </row>
    <xsl:apply-templates select="UML:Classifier.feature/
                         UML:Attribute" />
  </xsl:if>
</xsl:template>


<xsl:template match="UML:Attribute">
  <xsl:variable name="target"
        select='@type'/>


  <row>
      <entry role="feature-detail">
      <xsl:value-of select="@visibility"/>
      </entry>

      <entry role="feature-detail">
      <xsl:call-template name="classify">
          <xsl:with-param name="target" select="$target" />
      </xsl:call-template>
      </entry>

      <entry role="feature-detail">
      <xsl:value-of select="@name"/>
      </entry>
  </row>

  <row>
      <entry role="comment"/>
      <entry namest="c2" nameend="c3" role="comment"><xsl:value-of select="@comment"/></entry>
  </row>

</xsl:template>



<!-- Operations -->
<xsl:template name="operations">
  <xsl:variable name="class_operations" 
                select="UML:Classifier.feature/UML:Operation" />
  <xsl:if test="count($class_operations) > 0">
    <row>
      <entry role="info-title" namest="c1" nameend="c3" ><para>Operations:</para></entry>
    </row>
    <row>
      <entry role="feature-heading" ><para>visibility</para></entry>
      <entry role="feature-heading" ><para>return</para></entry>
      <entry role="feature-heading" ><para>name</para></entry>
    </row>

    <xsl:apply-templates select="UML:Classifier.feature/
                                 UML:Operation" />
  </xsl:if>
</xsl:template>


<xsl:template match="UML:Operation">

    <xsl:variable name="parameters"
         select="UML:BehavioralFeature.parameter/
         UML:Parameter[@kind!='return']" />
	 
    <xsl:variable name="return"
         select="UML:BehavioralFeature.parameter/
         UML:Parameter[@kind='return']" />
		  
    <xsl:variable name="target"
         select="$return/@type" />

    <row>
        <entry role="feature-detail">
            <xsl:value-of select="@visibility" />
        </entry>

        <entry role="feature-detail">

        <xsl:choose>
            <xsl:when test="string-length($target) = 0">
                <para role="datatype"><xsl:text>void</xsl:text></para>
            </xsl:when>

            <xsl:otherwise>
                <xsl:call-template name="classify">
                    <xsl:with-param name="target" select="$target" />
                </xsl:call-template>
            </xsl:otherwise>
        </xsl:choose>

        </entry>

        <entry role="feature-detail">
            <xsl:value-of select="@name"/>
        </entry>


    </row>

    <row>
        <entry role="comment"/>
        <entry namest="c2" nameend="c3" role="comment"><xsl:value-of select="@comment"/></entry>
    </row>

    <xsl:variable name="parameter-count" select="count($parameters)" />

    <xsl:if test="not(normalize-space($parameter-count)='0')">

    <row>
        <entry role="feature-detail" >
            <xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text>
        </entry>
        
        <entry role="parameter-heading" valign="top">parameters:</entry>
        <entry bgcolor="#ffffff" align="right">
            <!--table width="85%" align="right" cellpadding="0" cellspacing="0" border="0">
                <xsl:apply-templates select="$parameters" />
            </table-->
        </entry>
    </row>

    </xsl:if >

</xsl:template>



<!-- Parameter -->
<xsl:template match="UML:Parameter">
    <xsl:variable name="target" 
         select="@type" />

    <row>
        <entry role="feature-detail" width="45%">
            <xsl:call-template name="classify">
                <xsl:with-param name="target" select="$target" />
            </xsl:call-template>
        </entry>

        <entry role="feature-detail" width="55%">
            <xsl:value-of select="@name"/>
        </entry>
    </row>
</xsl:template>


<!-- Classification -->
<xsl:template name="classify">
    <xsl:param name="target"/>

    <xsl:variable name="classifier"
         select="key('classifier', $target)" />
                  
    <xsl:variable name="classifier_name"
         select="$classifier/@name" />

    <xsl:variable name="type" select="name($classifier)" />

    <!-- Get the type of the classifier (class, interface, datatype) -->
    <xsl:variable name="classifier_type">
    <xsl:choose>
        <xsl:when test="$type='UML:Class'">classifier</xsl:when>
        <xsl:when test="$type='UML:Interface'">interface</xsl:when>
        <xsl:when test="$type='UML:DataType'">datatype</xsl:when>
        <xsl:otherwise>classifier</xsl:otherwise>
    </xsl:choose>
    </xsl:variable>
    
    <xsl:choose>
        <!-- Datatypes don't have hyperlinks -->
        <xsl:when test="$type='UML:DataType'">
            <span role="datatype">
                <xsl:value-of select="$classifier_name"/>
            </span>
        </xsl:when>
        
        <!-- Classes and Interfaces have hyperlinks -->
        <!-- The classifier type is used to style appropriately -->
        <xsl:otherwise>
            <xsl:if test="string-length($classifier) > 0">
                <!--a role={$classifier_type}" href="#{$classifier_name}"-->
                <xsl:value-of select="$classifier_name"/>
                <!--/a-->
            </xsl:if>
        </xsl:otherwise>
    </xsl:choose>

</xsl:template>

<!--xsl:template match="XMI.content"> 
  
  <xsl:comment >maintitlebegin1&title1;maintitleend1</xsl:comment>   
  <xsl:comment >maintitlebegin2&title2;maintitleend2</xsl:comment>   
  <xsl:apply-templates select="diagrams/diagram"/>    
</xsl:template--> 


  <xsl:template name="actor"> 
    <xsl:param name = "idvalue"><xsl:value-of select="@xmi.id"/></xsl:param>
    <!--td valign="top"--> 
      <para>
        <!--role="push"-->
      <xsl:for-each select="//XMI.content/umlobjects/UML:Actor">
        <xsl:if test="@xmi.id = $idvalue">
          <emphasis><xsl:value-of select="@name"/>:</emphasis>
          <xsl:value-of select="@documentation"/>
        </xsl:if>
      </xsl:for-each > 
      </para>
    <!--/td-->
  </xsl:template>
  
  <xsl:template name="usecase"> 
    <xsl:param name = "idvalue"><xsl:value-of select="@xmi.id"/></xsl:param>
    <!--td valign="top"--> 
      <para>
        <!--role="push"-->
        <xsl:for-each select="//XMI.content/umlobjects/UML:UseCase"> 
          <xsl:if test="@xmi.id = $idvalue">
            <emphasis><xsl:value-of select="@name"/>:</emphasis>
            <xsl:value-of select="@documentation"/>
            
          </xsl:if>
        </xsl:for-each > 
      </para>
    <!--/td-->
  </xsl:template>
  
  <xsl:template name="class"> 
    <xsl:param name = "idvalue"><xsl:value-of select="@xmi.id"/></xsl:param>
    <!--td valign="top"--> 
      <div role="push">
        <xsl:for-each select="//XMI.content/umlobjects/UML:Class">
          <xsl:if test="@xmi.id = $idvalue">
            <div role="boldtext">&packagename;<xsl:value-of select="@package"/></div>
            <div role="boldtext">&classname;<xsl:value-of select="@name"/></div>
            <xsl:value-of select="@documentation"/>
            <div role="push">
        
              Stereotype: <xsl:value-of select="@stereotype"/>
              
              Abstarct: 
              <xsl:if test="@abstract='1'">
              <xsl:text>&yes;</xsl:text>
              </xsl:if>
              <xsl:if test="@abstract='0'">
              <xsl:text>&no;</xsl:text>
              </xsl:if>
              
              Visibility: 
              <xsl:if test="@scope='200'">
              <xsl:text>public</xsl:text>
              </xsl:if>
              <xsl:if test="@scope='201'">
              <xsl:text>private</xsl:text>
              </xsl:if>
              <xsl:if test="@scope='202'">
              <xsl:text>protected</xsl:text>
              </xsl:if>
              
              <xsl:call-template name="attribute"/> 
              <xsl:call-template name="operation"/> 
            </div>
          </xsl:if>
        </xsl:for-each > 
      </div>
    <!--/td-->
  </xsl:template>
  

  <xsl:template name="attribute"> 
    <!--td valign="top"--> 
      <div role="boldtext">&attributes;</div>
      
      <xsl:for-each select="UML:Attribute">
        <xsl:value-of select="@name"/><xsl:text> - </xsl:text><xsl:value-of select="@type"/>
        Static: 
        <xsl:if test="@static='1'">
          <xsl:text>&yes;</xsl:text>
        </xsl:if>
        <xsl:if test="@static='0'">
          <xsl:text>&no;</xsl:text>
        </xsl:if>
        
        Visibility: 
        <xsl:if test="@scope='200'">
          <xsl:text>public</xsl:text>
        </xsl:if>
        <xsl:if test="@scope='201'">
          <xsl:text>private</xsl:text>
        </xsl:if>
        <xsl:if test="@scope='202'">
          <xsl:text>protected</xsl:text>
        </xsl:if>
        
        Default �t�: <xsl:value-of select="@value"/>
        
        <xsl:value-of select="@documentation"/>
        
      </xsl:for-each > 
    <!--/td-->
  </xsl:template>
  
  <xsl:template name="operation"> 
    <entry valign="top"> 
      <div role="boldtext">&metodes;</div>
      
      <xsl:for-each select="UML:Operation">
        <i>
          <xsl:if test="@abstract='1'">
            <xsl:text>abstract </xsl:text>
          </xsl:if>   
          <xsl:if test="@scope='200'">
            <xsl:text>public </xsl:text>
          </xsl:if>
          <xsl:if test="@scope='201'">
            <xsl:text>private </xsl:text>
          </xsl:if>
          <xsl:if test="@scope='202'">
            <xsl:text>protected </xsl:text>
          </xsl:if>
          
          <xsl:value-of select="@type"/><xsl:text> </xsl:text>
          
          <xsl:value-of select="@name"/><xsl:text>(</xsl:text>
          
          <xsl:apply-templates select="UML:Parameter" mode="diagram"/>
          <xsl:text>)</xsl:text>
          
        </i>
        <xsl:value-of select="@documentation"/>  
        &parameters;
        <xsl:for-each select="UML:Parameter"> 
        <div role="push">
          <xsl:value-of select="@type"/>
          <xsl:text> </xsl:text>
          <xsl:value-of select="@name"/>
          
          <xsl:value-of select="@documentation"/>
          
        </div>
        </xsl:for-each> 
        
      </xsl:for-each > 
    </entry>
  </xsl:template>
  
  <xsl:template match="UML:Parameter" mode="diagram"> 
    
    <xsl:value-of select="@type"/>
    <xsl:text> </xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:if test="position()!=last()">
      <xsl:text>, </xsl:text>
    </xsl:if>
    
  </xsl:template>  
</xsl:stylesheet>
