# dbusxml-to-md-xslt-template

D-Bus XML introspection to Markdown document.

Powered by [XSLT](https://gitlab.gnome.org/GNOME/libxslt/-/wikis/home).

```shell
xsltproc --novalid template.xsl ${xml-path} > ${doc-path}
```

Optional language parameter (default: `en_US`):

```shell
xsltproc --novalid --stringparam lang en_US template.xsl ${xml-path} > ${doc-path}
xsltproc --novalid --stringparam lang ru_RU template.xsl ${xml-path} > ${doc-path}
```

Parameter `lang` selects `doc:summary`/`doc:detail` with matching `xml:lang`. For `en_US` it falls back to nodes without `xml:lang` if no `en_US` version exists.

[Example input interface](example/org.freedesktop.example.example1.xml)

[Example output document](example/org.freedesktop.example.example1.md)
