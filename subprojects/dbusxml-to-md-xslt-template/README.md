# dbusxml-to-md-xslt-template

D-Bus XML introspection to Markdown document.

Powered by [XSLT](https://gitlab.gnome.org/GNOME/libxslt/-/wikis/home).

```shell
xsltproc --novalid template.xsl ${xml-path} > ${doc-path}
```

[Example input interface](example/org.freedesktop.example.example1.xml)

[Example output document](example/org.freedesktop.example.example1.md)
