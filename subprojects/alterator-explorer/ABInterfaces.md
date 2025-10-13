# Добавление модулей Alterator Explorer

Для корректной работы Alterator Explorer необходимы пакеты `alterator-manager` и `alterator-module-executor`.

## Интерфейс `org.altlinux.alterator.object` для добавления модулей

Чтобы быть отобажённым в Alterator Explorer модуль должен быть зарегистрирован на D-Bus, как один из подобъектов объекта `org.altlinux.alterator`, путём добавления .backend-файла в директорию `/usr/share/alterator/backends`, и реализовывать интерфейс `org.altlinux.alterator.object` с методом `info`. Ниже можно увидеть пример такого .backend-файла:

```
[Manager]
module_name = executor
node_name = adtapp

interface_name = application

[info]
execute = cat /usr/share/alterator/module-info/adt-app.desktop
stdout_strings = enabled
```

Более подробно с форматом .backend-файлов можно ознакомиться в [репозитории проекта alterator-manager](https://gitlab.basealt.space/alt/alterator-manager/-/blob/master/docs/README-ru.md).

Метод `info` должен возвращать информацию о модуле в формате [`X-Alterator Entry`](#x\-alterator-entry). Также потдерживается, но не рекомендуется формат `Desktop entry`: 

```
[Desktop Entry]
Type=Application
Categories=X-Alterator-System
Icon=net-wifi
Terminal=false
Name=Alt Diagnostic Tool
Name[ru]=Инструмент диагностики

Name[ru_RU]=adt-app.desktop
```

## Интерфейс `org.altlinux.alterator.category` для описания категорий модулей

Данные о категории можно получить с помощью интерфейса `org.altlinux.alterator.category`, метод `info` которого возвращает данные о категории по её названию (например `X-Alterator-System`) в формате `X-Alterator Entry` или .directory-файла:

```
[Desktop Entry]
Type=Directory
Name=System
Comment=Operating system information and setup
Icon=groups/system
X-Alterator-Category=X-Alterator-System
Name[ru]=Система
Name[uk]=Система
Name[pt_BR]=Sistema
Name[es]=Sistema
Comment[ru]=Информация об установленной системе и её настройка
Comment[uk]=Налаштування операційної системи та інформація щодо неї
Comment[pt_BR]=Informações e Configurações do Sistema operacional
Comment[es]=Información del sistema operativo y la configuración
```

Для хранения этих файлов предусмотренны директории `/usr/share/alterator/module-info/` и `/usr/share/alterator/desktop-directories/`.

### X-Alterator Entry

.desktop-файл описывающий некую сущность связанную с Альтератором должен содержать одну и только одну секцию `[X-Alterator Entry]` в качестве входной точки, он также может содержать `Desktop entry` и любые другие секции связанные или не связанные с Альтератором. `X-Alterator Entry` содержит перечисление всех X-Alterator сущностей содержащихся в данном .desktop-файле, например:

```
[Desktop Entry]
Name=Something
Categories=Somewhere
Implements=Somewhat
Exec=Somehow

[X-Alterator Entry]
Implements=i1; i2; i3
Category=c1; c2
Object=o1; o2; o3; o4
...
```

Каждый из перечисленных элементов должен быть описан, в этом же файле, в отдельной секции вида `[X-Alterator <Entity> <Entity_Name>]`, где &lt;Entity&gt; обозначает тип описываемой сущности, а &lt;Entity_Name&gt; - её имя, например:

```
[X-Alterator Object o1]
Name=o1
Name[ru]=об1
...
```

На данный момент поддерживаются следующие сущности:

- Interface
- Object
- Category
- To be continued...

#### X-Alterator Interface

Для перечисления D-Bus интерфейсов в `[X-Alterator Entry]` используется ключ `Implements`. Секция должна иметь название соответствующее шаблону: `[X-Alterator Interface <name>]`, где &lt;name&gt; обозначает полное имя интерфейса на D-Bus, например: `org.altlinux.alterator.diag1`. Секция интерфейса должна содержать следующие поля:

- Exec - команда запускающая прииложение реализующее интерфейсом, вида: &lt;app&gt; &lt;params&gt; %&lt;code&gt;, где &lt;code&gt; - специальный код обозначающий возможность передачи дополнительного параметра, сущетвуют следующие коды:
	- %o - указывает на возможность передачи в пути D-Bus объекта в качестве параметра

#### X-Alterator Object

Сущности типа `Object` перечисляются в поле `Objects`. Соответствующяа объекту секция должна иметь имя `[X-Alterator Object <name>]`, &lt;name&gt; должно отражать полный путь D-Bus объекта. Описание объекта должно содержать ключи:

- Name (возможно с переводами) - имя объекта
- Icon - путь к файлу иконки
- Comment (возможно с переводами) - описание объекта
- CategoryInternalName - внутреннее имя для категории к которой отнесён этот объект

#### X-Alterator Category

Для списка категорий в `[X-Alterator Entry]` выделен ключ `Categories`. Секция категории должна называться аналогично другим `[X-Alterator Object <name>]`, где в &lt;name&gt; записано `CategoryInternalName` данной категории. Категория обладает такими свойствами, как:

- Name (возможно с переводами) - читаемое для человека имя категории
- Icon - путь к файлу иконки
- Comment (возможно с переводами) - описание категории

## Интерфейс добавления интерфейсов модулей

Для того чтобы приложение реализующее интерфейс модуля альтератора было обнаружено Alterator Explorer\`ом его .desktop-файл расположенный в `/usr/share/applications`, должен содержать секцию `[X-Alterator]` и имя реализуемого данным приложением интерфейса вычитывается из свойства `interface`.

```
[X-Alterator]
Interface=org.altlinux.alterator.diag1
```

Также `[Desktop Entry]` .desktop-файла должна содержать запись `Exec`.
