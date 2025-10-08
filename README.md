- [1. Alterator Explorer](#1-alterator-explorer)
	- [1.1. Что такое модуль?](#11-что-такое-модуль)
	- [1.2. Как создать модуль?](#12-как-создать-модуль)
		- [1.2.1. Регистрация на D-Bus](#121-регистрация-на-d-bus)
	- [1.3. Категории модулей](#13-категории-модулей)
	- [1.4. Интерфейс добавления интерфейсов приложений](#14-интерфейс-добавления-интерфейсов-приложений)
- [2. Спецификация интерфейса `org.altlinux.alterator.legacy1`](#2-спецификация-интерфейса-rubasealtalteratorlegacy1)
	- [2.1. Интерфейсы org.altlinux.alterator.legacyX](#21-интерфейсы-rubasealtalteratorlegacyx)
		- [2.1.1. Специцикация интерфейса org.altlinux.alterator.application1](#211-специцикация-интерфейса-rubasealtalteratorapplication1)
			- [2.1.1.1. Методы](#2111-методы)
				- [2.1.1.1.1. Info](#21111-info)
				- [2.1.1.1.2. Run](#21112-run)
			- [2.1.1.2. Сигналы](#2112-сигналы)
- [3. Примеры модифицированных Desktop Entry](#3-примеры-модифицированных-desktop-entry)
	- [3.1. Модуль ACC](#31-модуль-acc)
	- [3.2. Категория ACC](#32-категория-acc)


# 1. Alterator Explorer

**Alterator Explorer** &mdash;  это графическое приложение для поиска и вызова системных утилит, называемых *модулями*. Он предназначен для просмотра и запуска приложений для использования функционала объектов на D-Bus, на замену Alterator Control Center.

Основным функционалом является поиск (с помощью Alterator Manager и его модуля Executor), объектов на системной шине D-Bus, реализующих различные интерфейсы, например: org.altlinux.alterator.diag1 &mdash;  объекты диагностических инструментов, org.altlinux.alterator.component1 &mdash; объекты компонентов операционной системы и другие. Если **Alterator Explorer** находит необходимое приложение для использования функционала объектов, то можно этот объект «запустить». Например, для использования объекта диагностического инструмента с интерфейсом org.altlinux.alterator.diag1 необходимо приложение ADT.

Интерфейс org.altlinux.alterator.legacy используется для запуска модулей Alterator Control Center. Также, есть возможность запустить сам **Alterator Control Center**.

В разработке находятся модули для управления компонентами и пакетами.

## 1.1. Что такое модуль? 
Модулем может быть как графическое приложение, так и простой скрипт. Каждый модуль представлен своим объектом на D-Bus, через который он и будет обнаружен Alterator Explorer. Объект модуля на D-Bus предоставляет доступ к методам утилит через свои интерфейсы. В качестве графических интерфейсов для утилит используются локально установленные приложения соответствующие [некоторым
условиям](#15-интерфейс-добавления-интерфейсов-приложений).


## 1.2. Как создать модуль? 

Каждый модуль должен быть зарегистрирован на D-Bus как объект сервиса `/org/altlinux/alterator` и реализовывать один из поддерживаемых интерфейсов. Каждый интерфейс отражает тот или иной тип модуля Альтератора. Для каждого типа модуля существует сущность формата [Alterator Entry](https://gitlab.basealt.space/alt/alterator-entry/doc/README.md). На данный момент поддерживаются следующие интерфейсы:

| Интерфейс | Описание | Сущность Alterator Entry |
|:-|-|-|
|[org.altlinux.alterator.legacy1](#2-спецификация-интерфейса-rubasealtalteratorlegacy1)|Legacy-модуль для Alterator Control Center|Используется [модифицированный формат Desktop Entry](#31-модуль-acc)|
|[org.altlinux.alterator.diag1](<!--TODO(kozyrevid): add interface spec and link to it-->)|Диагностический инструмент|Diag или Diagnostictool|
|[org.altlinux.alterator.apt1](<!--TODO(kozyrevid): add interface spec and link to it-->)|Модуль управления пакетами при помощи apt|Object|
|[org.altlinux.alterator.rpm1](<!--TODO(kozyrevid): add interface spec and link to it-->)|Модуль управления пакетами при помощи rpm|Object|
|[org.altlinux.alterator.repo1](<!--TODO(kozyrevid): add interface spec and link to it-->)|Модуль управления репозиториями apt|Object|
|[org.altlinux.alterator.componentsApp1](<!--TODO(kozyrevid): add interface spec and link to it-->)|Модуль управления компонентами операционной системы|Object|

### 1.2.1. Регистрация на D-Bus 

Зарегистрировать модуль на D-Bus можно разными способами, но мы рассмотрим регистрацию путём добавления `.backend` файла в `/etc/alterator/backends` или `/usr/share/alterator/backends`. Данные файлы должны содержать [Alterator Entry](https://gitlab.basealt.space/alt/alterator-entry/doc/README.md) типа `Backend`.

Каждый `.backend` файл описывает ровно один D-Bus-интерфейс, для создания объекта со множеством интерфейсов придётся создать несколько файлов. 

На основании этих файлов, alterator-manager, при помощи одного из вспомогательных модулей (например, alterator-module-executor), регистрирует на D-Bus сервисе `/org/altlinux/alterator` объекты с указанными интерфейсами.  Более подробно с возможностами alterator-manager можно ознакомиться в [репозитории проекта](https://gitlab.basealt.space/alt/alterator-manager/-/blob/master/docs/README-ru.md).

## 1.3. Категории модулей 

Alterator Explorer отображает модули по категориям, данные о категориях он получает при помощи интерфейса [`org.altlinux.alterator.actegories1`](https://gitlab.basealt.space/alt/alterator-module-categories/-/blob/master/README.md) в формате [Alterator Entry](https://gitlab.basealt.space/alt/alterator-entry/doc/README.md) типа `Category`. Список доступных категорий является расширяемым, для добавления новой категории необходимо поместить файл `<category_name>.category` в директорию `/usr/share/alterator/categories`. Также существуют *legacy*-категории, которые хранятся в директории `/usr/share/alterator/desktop-directories` в [модифицированном формате Desktop Entry](#31-категория-acc), однако не рекомендуется измеменять *legacy*-категории или добавлять новые.

## 1.4. Интерфейс добавления интерфейсов приложений 

Для того чтобы приложение реализующее интерфейс модуля альтератора было обнаружено Alterator Explorer его `.desktop` файл расположенный в `/usr/share/applications`, должен содержать секцию [`[X-Alterator Entry]`](#X-Alterator_Entry "wikilink") с единственной записью в единственном ключе `Applications`, соответствующей данному приложению.

# 2. Спецификация интерфейса `org.altlinux.alterator.legacy1` 	

## 2.1. Интерфейсы org.altlinux.alterator.legacyX

Интерфейсы `legacy` предназначены для предоставления возможности запуска модулей Alterator Control Center посредством D-Bus-интерфейсов.

### 2.1.1. Специцикация интерфейса org.altlinux.alterator.application1

Данный интерфейс должен быть реализован для каждого модуля Alterator Control Center. 

Все реализации интерфейса должны обладать **исключительно** методами и сигналами описанными в данной спецификации. Сигнатуры методов и сигналов также должы полностью совпадать с описанноми в спецификации. В обратном случае валидация данной реализации провалится и экземпляр интерфейса **не будет зарегистрирован** на D-Bus.

#### 2.1.1.1. Методы 

##### 2.1.1.1.1. Info

Метод `Info () -> (Array of [Byte] stdout_bytes, Int32 response)` предназначен для предоставления информации о модуле, представленном данным объектом, в модифицированном формате [Desktop Entry](https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html). Результатом выполнения метода является пара массива байтов `stdout_bytes`, который содержит о модуле, и целочисленного числа `response`, отражающего код возврата метода. 

##### 2.1.1.1.2. Run

Метод `Run () -> (Array of [String] stderr_strings, Int32 response)` применяется приложением alterator-legacy-runner, которое позволяет запускать модули старого альтератора в *stanalone* режиме при помощи D-Bus. Выходные аргументы `stderr_strings` и `response` предназначены для передачи возможных сообщений об ошибках и кода возврата, соответственно. 

#### 2.1.1.2. Сигналы

Интерфейс не допускает ни каких сигналов.

# 3. Примеры модифицированных Desktop Entry

Desktop Entry &mdash; формат конфигурционных файлов, используемый многими средами рабочего стола, с подробной спецификацией формата можно ознакомиться на [сайте проекта](https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html).

Alterator Control Center(ACC) использовал модификацию формата Desktop Entry для хранения данных о разных своих составляющих. Alterator Explorer обладает функционалом позволяющим использовать модули и категории Alterator Control Center в качестве *legacy*-сущностей. Данная глава содержит примеры *legacy*-файлов для модулей и категорий.

## 3.1. Модуль ACC

В качестве примера приведено содержимое файла `/usr/share/alterator/applications/auth.desktop`, описывающего модуль ACC `auth`.

```
[Desktop Entry]
Type=Application
Categories=X-Alterator-Users
Icon=auth
Terminal=false
Name=Authentication
X-Alterator-URI=/auth
X-Alterator-Weight=20
X-Alterator-Help=auth
Name[ru]=Аутентификация
Name[uk]=Аутентифікація
Name[pt_BR]=Autentificaçãoq
```

## 3.2. Категория ACC

В качестве примера приведено содержимое файла `/usr/share/alterator/desktop-directories/accounting.directory`, описывающего категорию ACC `accounting`.

```
[Desktop Entry]
Type=Directory
Name=Accounting
Comment=System accounting
Icon=groups/accounting
X-Alterator-Category=X-Alterator-Accounting
Name[ru]=Учёт
Name[uk]=Облік
Name[pt_BR]=Contabilidade
Name[es]=Contabilidade
Comment[ru]=Учётная информация по системе
Comment[uk]=Системний облік
Comment[pt_BR]=Sistema de Contabilidade
Comment[es]=Cuenta de Sistema
```
