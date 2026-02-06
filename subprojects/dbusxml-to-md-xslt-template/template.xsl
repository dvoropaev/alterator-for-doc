<?xml version="1.0"?>
<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:doc="http://www.freedesktop.org/dbus/1.0/doc.dtd"
    xmlns:xml="http://www.w3.org/XML/1998/namespace">

    <xsl:param name="lang" select="'en'"/>

    <xsl:output method="text" encoding="UTF-8"/>
	
    <!-- ROOT TEMPLATE -->
    <xsl:template match="/node">
        <xsl:apply-templates select="interface"/>
		<xsl:if test="interface"><xsl:text>&#10;</xsl:text></xsl:if>
    </xsl:template>

    <xsl:template name="tr">
        <xsl:param name="key"/>
        <xsl:choose>
            <xsl:when test="$lang = 'ru' or $lang = 'ru_RU'">
                <xsl:choose>
                    <xsl:when test="$key = 'interface'">Интерфейс</xsl:when>
                    <xsl:when test="$key = 'methods'">Методы</xsl:when>
                    <xsl:when test="$key = 'signals'">Сигналы</xsl:when>
                    <xsl:when test="$key = 'method'">Метод</xsl:when>
                    <xsl:when test="$key = 'signal'">Сигнал</xsl:when>
                    <xsl:when test="$key = 'summary'">Описание</xsl:when>
                    <xsl:when test="$key = 'input_arguments'">Входные аргументы</xsl:when>
                    <xsl:when test="$key = 'output_arguments'">Выходные аргументы</xsl:when>
                    <xsl:when test="$key = 'argument'">Аргумент</xsl:when>
                    <xsl:otherwise><xsl:value-of select="$key"/></xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:otherwise>
                <xsl:choose>
                    <xsl:when test="$key = 'interface'">Interface</xsl:when>
                    <xsl:when test="$key = 'methods'">Methods</xsl:when>
                    <xsl:when test="$key = 'signals'">Signals</xsl:when>
                    <xsl:when test="$key = 'method'">Method</xsl:when>
                    <xsl:when test="$key = 'signal'">Signal</xsl:when>
                    <xsl:when test="$key = 'summary'">Summary</xsl:when>
                    <xsl:when test="$key = 'input_arguments'">Input arguments</xsl:when>
                    <xsl:when test="$key = 'output_arguments'">Output arguments</xsl:when>
                    <xsl:when test="$key = 'argument'">Argument</xsl:when>
                    <xsl:otherwise><xsl:value-of select="$key"/></xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="summary-node">
        <xsl:choose>
            <xsl:when test="doc:summary[@xml:lang=$lang]">
                <xsl:apply-templates select="doc:summary[@xml:lang=$lang][1]"/>
            </xsl:when>
            <xsl:when test="doc:summary[not(@xml:lang)]">
                <xsl:apply-templates select="doc:summary[not(@xml:lang)][1]"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:apply-templates select="doc:summary[1]"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="detail-nodes">
        <xsl:choose>
            <xsl:when test="doc:detail[@xml:lang=$lang]">
                <xsl:apply-templates select="doc:detail[@xml:lang=$lang]"/>
            </xsl:when>
            <xsl:when test="doc:detail[not(@xml:lang)]">
                <xsl:apply-templates select="doc:detail[not(@xml:lang)]"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:apply-templates select="doc:detail"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <!-- INTERFACE TEMPLATE -->
    <xsl:template match="interface"># <xsl:call-template name="tr"><xsl:with-param name="key" select="'interface'"/></xsl:call-template> **<xsl:value-of select="@name"/>**<xsl:text>&#10;</xsl:text>
		<xsl:if test="doc:summary"><xsl:text>&#10;</xsl:text></xsl:if>
		<xsl:call-template name="summary-node"/>
		<xsl:if test="doc:detail"><xsl:text>&#10;</xsl:text></xsl:if>
		<xsl:call-template name="detail-nodes"/>
		<xsl:if test="method">
			<xsl:call-template name="methods-navigation-table"/>
		</xsl:if>
		<xsl:if test="signal">
			<xsl:call-template name="signals-navigation-table"/>
		</xsl:if>
		<xsl:if test="method">
			<xsl:text>&#10;</xsl:text>
			<xsl:text>## </xsl:text><xsl:call-template name="tr"><xsl:with-param name="key" select="'methods'"/></xsl:call-template><xsl:text>&#10;</xsl:text>
			<xsl:text>&#10;</xsl:text>
		</xsl:if>
        <xsl:apply-templates select="method"/>
		<xsl:if test="signal">
			<xsl:text>&#10;## </xsl:text><xsl:call-template name="tr"><xsl:with-param name="key" select="'signals'"/></xsl:call-template><xsl:text>&#10;&#10;</xsl:text>
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

    <xsl:template name="methods-navigation-table">
| <xsl:call-template name="tr"><xsl:with-param name="key" select="'method'"/></xsl:call-template> | <xsl:call-template name="tr"><xsl:with-param name="key" select="'summary'"/></xsl:call-template> |
|--------|---------|<xsl:for-each select="method">
| [<xsl:value-of select="@name"/>]<xsl:call-template name="method-link"><xsl:with-param name="method_name" select="@name"/><xsl:with-param name="interface_name" select="''"/></xsl:call-template> | <xsl:value-of select="normalize-space((doc:summary[@xml:lang=$lang] | doc:summary[not(@xml:lang)] | doc:summary)[1])"/> |</xsl:for-each>
		<xsl:text>&#10;&#10;</xsl:text>
	</xsl:template>
	
	<xsl:template name="signals-navigation-table">
| <xsl:call-template name="tr"><xsl:with-param name="key" select="'signal'"/></xsl:call-template> | <xsl:call-template name="tr"><xsl:with-param name="key" select="'summary'"/></xsl:call-template> |
|--------|---------|<xsl:for-each select="signal">
| [<xsl:value-of select="@name"/>]<xsl:call-template name="signal-link"><xsl:with-param name="signal_name" select="@name"/><xsl:with-param name="interface_name" select="''"/></xsl:call-template> | <xsl:value-of select="normalize-space((doc:summary[@xml:lang=$lang] | doc:summary[not(@xml:lang)] | doc:summary)[1])"/> |</xsl:for-each>
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
		<xsl:if test="doc:summary"><xsl:text>&#10;</xsl:text></xsl:if>
        <xsl:call-template name="summary-node"/>
		<xsl:if test="doc:detail"><xsl:text>&#10;</xsl:text></xsl:if>
		<xsl:call-template name="detail-nodes"/>
		<xsl:if test="arg[@direction='in']">
			<xsl:text>&#10;#### </xsl:text><xsl:call-template name="tr"><xsl:with-param name="key" select="'input_arguments'"/></xsl:call-template><xsl:text>&#10;&#10;</xsl:text>
		</xsl:if>
		<xsl:apply-templates select="arg[@direction='in']">
			<xsl:with-param name="method_name" select="@name"/>
		</xsl:apply-templates>
		<xsl:if test="arg[@direction='out']">
			<xsl:text>&#10;#### </xsl:text><xsl:call-template name="tr"><xsl:with-param name="key" select="'output_arguments'"/></xsl:call-template><xsl:text>&#10;&#10;</xsl:text>
		</xsl:if>
		<xsl:apply-templates select="arg[@direction='out']">
			<xsl:with-param name="method_name" select="@name"/>
		</xsl:apply-templates>
        <xsl:if test="position() != last()"><xsl:text>&#10;</xsl:text></xsl:if>
    </xsl:template>

    <!-- SIGNAL TEMPLATE -->
    <xsl:template match="signal">
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
		<xsl:if test="doc:summary"><xsl:text>&#10;</xsl:text></xsl:if>
        <xsl:call-template name="summary-node"/>
		<xsl:if test="doc:detail"><xsl:text>&#10;</xsl:text></xsl:if>
		<xsl:call-template name="detail-nodes"/>
		<xsl:if test="arg"><xsl:text>&#10;#### </xsl:text><xsl:call-template name="tr"><xsl:with-param name="key" select="'output_arguments'"/></xsl:call-template><xsl:text>&#10;&#10;</xsl:text></xsl:if>
		<xsl:apply-templates select="arg">
			<xsl:with-param name="method_name" select="@name"/>
		</xsl:apply-templates>
        <xsl:if test="position() != last()"><xsl:text>&#10;</xsl:text></xsl:if>
    </xsl:template>

	<!-- ARGUMENT TEMPLATE -->
	<xsl:template match="arg">
		<xsl:param name="method_name"/>##### <xsl:if test="string-length(@name) = 0"><xsl:call-template name="tr"><xsl:with-param name="key" select="'argument'"/></xsl:call-template><xsl:text> </xsl:text></xsl:if><xsl:if test="string-length(@name) != 0">**<xsl:value-of select="@name"/>** : </xsl:if>`<xsl:value-of select="@type"/>`<xsl:if test="string-length(@name) != 0"> &lt;a id="argument-<xsl:value-of select="@name"/>-of-<xsl:value-of select="$method_name"/>"&gt;&lt;/a&gt;</xsl:if><xsl:text>&#10;</xsl:text>
		<xsl:if test="doc:summary"><xsl:text>&#10;</xsl:text></xsl:if>
		<xsl:call-template name="summary-node"/>
		<xsl:if test="doc:detail"><xsl:text>&#10;</xsl:text></xsl:if>
		<xsl:call-template name="detail-nodes"/>
        <xsl:if test="position() != last()"><xsl:text>&#10;</xsl:text></xsl:if>
    </xsl:template>
</xsl:stylesheet>
