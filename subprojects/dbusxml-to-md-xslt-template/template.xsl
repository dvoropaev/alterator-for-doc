<?xml version="1.0"?>
<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:doc="http://www.freedesktop.org/dbus/1.0/doc.dtd">

    <xsl:output method="text" encoding="UTF-8"/>
    <xsl:param name="lang" select="'en'"/>
	
    <!-- ROOT TEMPLATE -->
    <xsl:template match="/node">
        <xsl:apply-templates select="interface"/>
		<xsl:if test="interface"><xsl:text>&#10;</xsl:text></xsl:if>
    </xsl:template>

    <!-- INTERFACE TEMPLATE -->
    <xsl:template match="interface"># Interface **<xsl:value-of select="@name"/>**<xsl:text>&#10;</xsl:text>
		<xsl:variable name="has_summary" select="($lang='en' and (doc:summary[@xml:lang='en'] or doc:summary[not(@xml:lang)])) or ($lang!='en' and doc:summary[@xml:lang=$lang])"/>
		<xsl:variable name="has_detail" select="($lang='en' and (doc:detail[@xml:lang='en'] or doc:detail[not(@xml:lang)])) or ($lang!='en' and doc:detail[@xml:lang=$lang])"/>
		<xsl:if test="$has_summary"><xsl:text>&#10;</xsl:text></xsl:if>
		<xsl:call-template name="render-doc-summary"/>
		<xsl:if test="$has_detail"><xsl:text>&#10;</xsl:text></xsl:if>
		<xsl:call-template name="render-doc-detail"/>
		<xsl:if test="method">
			<xsl:call-template name="methods-navigation-table"/>
		</xsl:if>
		<xsl:if test="signal">
			<xsl:call-template name="signals-navigation-table"/>
		</xsl:if>
		<xsl:if test="method">
			<xsl:text>&#10;</xsl:text>
			<xsl:text>## Methods&#10;</xsl:text>
			<xsl:text>&#10;</xsl:text>
		</xsl:if>
        <xsl:apply-templates select="method"/>
		<xsl:if test="signal">
			<xsl:text>&#10;## Signals&#10;&#10;</xsl:text>
		</xsl:if>
        <xsl:apply-templates select="signal"/>
        <xsl:if test="position() != last()"><xsl:text>&#10;</xsl:text></xsl:if>
    </xsl:template>

    <!-- DOC-SUMMARY TEMPLATE -->
    <xsl:template match="doc:summary">
        <xsl:for-each select="text()">
            <xsl:value-of select="translate(., '&#13;&#9;', '')"/>
            <xsl:text>&#10;</xsl:text>
        </xsl:for-each>
        <xsl:if test="position() != last()"><xsl:text>&#10;</xsl:text></xsl:if>
    </xsl:template>
	
	<!-- DOC-DETAIL TEMPLATE -->
	<xsl:template match="doc:detail">
		<xsl:variable name="txt" select="."/>
		<xsl:call-template name="clean-text">
			<xsl:with-param name="text" select="$txt"/>
		</xsl:call-template>
        <xsl:if test="position() != last()"><xsl:text>&#10;</xsl:text></xsl:if>
    </xsl:template>

	<!-- CLEAN TEXT TEMPLATE -->
	<xsl:template name="clean-text">
		<xsl:param name="text"/>
		<xsl:choose>
			<xsl:when test="contains($text, '&#10;')">
				<xsl:value-of select="normalize-space(substring-before($text, '&#10;'))"/>
				<xsl:text>&#10;</xsl:text>
				<xsl:call-template name="clean-text">
					<xsl:with-param name="text" select="substring-after($text, '&#10;')"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="normalize-space($text)"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="render-doc-summary">
		<xsl:choose>
			<xsl:when test="$lang='en'">
				<xsl:choose>
					<xsl:when test="doc:summary[@xml:lang='en']">
						<xsl:apply-templates select="doc:summary[@xml:lang='en']"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:apply-templates select="doc:summary[not(@xml:lang)]"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:otherwise>
				<xsl:apply-templates select="doc:summary[@xml:lang=$lang]"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="render-doc-detail">
		<xsl:choose>
			<xsl:when test="$lang='en'">
				<xsl:choose>
					<xsl:when test="doc:detail[@xml:lang='en']">
						<xsl:apply-templates select="doc:detail[@xml:lang='en']"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:apply-templates select="doc:detail[not(@xml:lang)]"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:otherwise>
				<xsl:apply-templates select="doc:detail[@xml:lang=$lang]"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="methods-navigation-table">
| Method | Summary |
|--------|---------|<xsl:for-each select="method">
<xsl:variable name="summary_text">
<xsl:choose>
<xsl:when test="$lang='en'">
<xsl:choose>
<xsl:when test="doc:summary[@xml:lang='en']"><xsl:value-of select="normalize-space(doc:summary[@xml:lang='en'])"/></xsl:when>
<xsl:otherwise><xsl:value-of select="normalize-space(doc:summary[not(@xml:lang)])"/></xsl:otherwise>
</xsl:choose>
</xsl:when>
<xsl:otherwise><xsl:value-of select="normalize-space(doc:summary[@xml:lang=$lang])"/></xsl:otherwise>
</xsl:choose>
</xsl:variable>
| [<xsl:value-of select="@name"/>]<xsl:call-template name="method-link"><xsl:with-param name="method_name" select="@name"/><xsl:with-param name="interface_name" select="''"/></xsl:call-template> | <xsl:value-of select="$summary_text"/> |</xsl:for-each>
		<xsl:text>&#10;&#10;</xsl:text>
	</xsl:template>
	
	<xsl:template name="signals-navigation-table">
| Signal | Summary |
|--------|---------|<xsl:for-each select="signal">
<xsl:variable name="summary_text">
<xsl:choose>
<xsl:when test="$lang='en'">
<xsl:choose>
<xsl:when test="doc:summary[@xml:lang='en']"><xsl:value-of select="normalize-space(doc:summary[@xml:lang='en'])"/></xsl:when>
<xsl:otherwise><xsl:value-of select="normalize-space(doc:summary[not(@xml:lang)])"/></xsl:otherwise>
</xsl:choose>
</xsl:when>
<xsl:otherwise><xsl:value-of select="normalize-space(doc:summary[@xml:lang=$lang])"/></xsl:otherwise>
</xsl:choose>
</xsl:variable>
| [<xsl:value-of select="@name"/>]<xsl:call-template name="signal-link"><xsl:with-param name="signal_name" select="@name"/><xsl:with-param name="interface_name" select="''"/></xsl:call-template> | <xsl:value-of select="$summary_text"/> |</xsl:for-each>
		<xsl:text>&#10;</xsl:text>
	</xsl:template>

	<xsl:template name="method-anchor-text">
		<xsl:param name="method_name"/>
		<xsl:param name="interface_name"/>
		<xsl:text>method-</xsl:text><xsl:value-of select="$method_name"/>
	</xsl:template>
	
	<xsl:template name="signal-anchor-text">
		<xsl:param name="signal_name"/>
		<xsl:param name="interface_name"/>
		<xsl:text>signal-</xsl:text><xsl:value-of select="$signal_name"/>
	</xsl:template>
	
	<xsl:template name="method-link">
		<xsl:param name="method_name"/>
		<xsl:param name="interface_name"/>
		<xsl:text>(#</xsl:text><xsl:call-template name="method-anchor-text">
			<xsl:with-param name="method_name" select="$method_name"/>
			<xsl:with-param name="interface_name" select="$interface_name"/>
		</xsl:call-template><xsl:text>)</xsl:text>
	</xsl:template>
	
	<xsl:template name="signal-link">
		<xsl:param name="signal_name"/>
		<xsl:param name="interface_name"/>
		<xsl:text>(#</xsl:text><xsl:call-template name="signal-anchor-text">
			<xsl:with-param name="signal_name" select="$signal_name"/>
			<xsl:with-param name="interface_name" select="$interface_name"/>
		</xsl:call-template><xsl:text>)</xsl:text>
	</xsl:template>

    <!-- METHOD TEMPLATE -->
    <xsl:template match="method">
		<xsl:variable name="method_name" select="@name"/>
		<xsl:variable name="has_summary" select="($lang='en' and (doc:summary[@xml:lang='en'] or doc:summary[not(@xml:lang)])) or ($lang!='en' and doc:summary[@xml:lang=$lang])"/>
		<xsl:variable name="has_detail" select="($lang='en' and (doc:detail[@xml:lang='en'] or doc:detail[not(@xml:lang)])) or ($lang!='en' and doc:detail[@xml:lang=$lang])"/>
        <xsl:text>### **</xsl:text><xsl:value-of select="@name"/><xsl:text>**(</xsl:text>
        <xsl:for-each select="arg[@direction='in']">
			<xsl:text>[</xsl:text><xsl:value-of select="@name"/>
			<xsl:text>](#argument-</xsl:text><xsl:value-of select="@name"/><xsl:text>-of-</xsl:text><xsl:value-of select="$method_name"/><xsl:text>) : `</xsl:text>
            <xsl:value-of select="@type"/><xsl:text>`</xsl:text>
            <xsl:if test="position() != last()">, </xsl:if>
        </xsl:for-each>
		<xsl:text>)</xsl:text>
        <xsl:text> -> (</xsl:text>
        <xsl:if test="arg[@direction='out']">
            <xsl:for-each select="arg[@direction='out']">
                <xsl:text>[</xsl:text><xsl:value-of select="@name"/>
				<xsl:text>](#argument-</xsl:text><xsl:value-of select="@name"/><xsl:text>-of-</xsl:text><xsl:value-of select="$method_name"/><xsl:text>) : `</xsl:text>
                <xsl:value-of select="@type"/><xsl:text>`</xsl:text>
                <xsl:if test="position() != last()">, </xsl:if>
            </xsl:for-each>
		</xsl:if>
        <xsl:text>)</xsl:text>
		<xsl:text>&lt;a id="</xsl:text><xsl:call-template name="method-anchor-text">
			<xsl:with-param name="method_name" select="@name"/>
			<xsl:with-param name="interface_name" select="''"/>
		</xsl:call-template>
		<xsl:text>"&gt;&lt;/a&gt;</xsl:text>
		<xsl:text>&#10;</xsl:text>
		<xsl:if test="$has_summary"><xsl:text>&#10;</xsl:text></xsl:if>
		<xsl:call-template name="render-doc-summary"/>
		<xsl:if test="$has_detail"><xsl:text>&#10;</xsl:text></xsl:if>
		<xsl:call-template name="render-doc-detail"/>
		<xsl:if test="arg[@direction='in']">
			<xsl:text>&#10;#### Input arguments&#10;&#10;</xsl:text>
		</xsl:if>
		<xsl:apply-templates select="arg[@direction='in']">
			<xsl:with-param name="method_name" select="@name"/>
		</xsl:apply-templates>
		<xsl:if test="arg[@direction='out']">
			<xsl:text>&#10;#### Output arguments&#10;&#10;</xsl:text>
		</xsl:if>
		<xsl:apply-templates select="arg[@direction='out']">
			<xsl:with-param name="method_name" select="@name"/>
		</xsl:apply-templates>
        <xsl:if test="position() != last()"><xsl:text>&#10;</xsl:text></xsl:if>
    </xsl:template>

    <!-- SIGNAL TEMPLATE -->
    <xsl:template match="signal">
		<xsl:variable name="has_summary" select="($lang='en' and (doc:summary[@xml:lang='en'] or doc:summary[not(@xml:lang)])) or ($lang!='en' and doc:summary[@xml:lang=$lang])"/>
		<xsl:variable name="has_detail" select="($lang='en' and (doc:detail[@xml:lang='en'] or doc:detail[not(@xml:lang)])) or ($lang!='en' and doc:detail[@xml:lang=$lang])"/>
        <xsl:text>### **</xsl:text><xsl:value-of select="@name"/><xsl:text>**</xsl:text>
        <xsl:text>(</xsl:text>
        <xsl:for-each select="arg">
			<xsl:text>`</xsl:text><xsl:value-of select="@type"/><xsl:text>`</xsl:text>
            <xsl:if test="position() != last()">, </xsl:if>
        </xsl:for-each>
        <xsl:text>)</xsl:text>
		<xsl:text>&lt;a id="</xsl:text><xsl:call-template name="signal-anchor-text">
			<xsl:with-param name="signal_name" select="@name"/>
			<xsl:with-param name="interface_name" select="''"/>
		</xsl:call-template>
		<xsl:text>"&gt;&lt;/a&gt;</xsl:text>
        <xsl:text>&#10;</xsl:text>
		<xsl:if test="$has_summary"><xsl:text>&#10;</xsl:text></xsl:if>
		<xsl:call-template name="render-doc-summary"/>
		<xsl:if test="$has_detail"><xsl:text>&#10;</xsl:text></xsl:if>
		<xsl:call-template name="render-doc-detail"/>
		<xsl:if test="arg"><xsl:text>&#10;#### Output arguments&#10;&#10;</xsl:text></xsl:if>
		<xsl:apply-templates select="arg">
			<xsl:with-param name="method_name" select="@name"/>
		</xsl:apply-templates>
        <xsl:if test="position() != last()"><xsl:text>&#10;</xsl:text></xsl:if>
    </xsl:template>

    <!-- ARGUMENT TEMPLATE -->
	<xsl:template match="arg">
		<xsl:param name="method_name"/>##### <xsl:if test="string-length(@name) = 0">Argument </xsl:if><xsl:if test="string-length(@name) != 0">**<xsl:value-of select="@name"/>** : </xsl:if>`<xsl:value-of select="@type"/>`<xsl:if test="string-length(@name) != 0"> &lt;a id="argument-<xsl:value-of select="@name"/>-of-<xsl:value-of select="$method_name"/>"&gt;&lt;/a&gt;</xsl:if><xsl:text>&#10;</xsl:text>
		<xsl:variable name="has_summary" select="($lang='en' and (doc:summary[@xml:lang='en'] or doc:summary[not(@xml:lang)])) or ($lang!='en' and doc:summary[@xml:lang=$lang])"/>
		<xsl:variable name="has_detail" select="($lang='en' and (doc:detail[@xml:lang='en'] or doc:detail[not(@xml:lang)])) or ($lang!='en' and doc:detail[@xml:lang=$lang])"/>
		<xsl:if test="$has_summary"><xsl:text>&#10;</xsl:text></xsl:if>
		<xsl:call-template name="render-doc-summary"/>
		<xsl:if test="$has_detail"><xsl:text>&#10;</xsl:text></xsl:if>
		<xsl:call-template name="render-doc-detail"/>
        <xsl:if test="position() != last()"><xsl:text>&#10;</xsl:text></xsl:if>
    </xsl:template>
</xsl:stylesheet>
