[English](./systeminfo1.md) | [Русский](./systeminfo1.ru_RU.md)

# Интерфейс **org.altlinux.alterator.systeminfo1**

Предоставляет методы бэкенда systeminfo для получения данных о системе через помощники `/usr/lib/alterator/backends/systeminfo`; каждый метод возвращает stdout и код завершения базовой команды.

| Метод | Описание |
|--------|------------|
| [GetAll](#method-GetAll) | Собирает данные о системе в виде пар ключ/значение. |
| [GetCPU](#method-GetCPU) | Возвращает модель CPU, число логических ядер и частоту, полученные через `systeminfo cpu`. |
| [GetHostName](#method-GetHostName) | Возвращает имя хоста, полученное через `systeminfo host-name`. |
| [GetDrive](#method-GetDrive) | Возвращает суммарный объём накопителей, полученный через `systeminfo drive`. |
| [GetGPU](#method-GetGPU) | Перечисляет VGA-адаптеры, сообщаемые `systeminfo gpu`. |
| [GetMemory](#method-GetMemory) | Сообщает объём RAM, полученный через `systeminfo memory`. |
| [GetBranch](#method-GetBranch) | Возвращает ветку дистрибутива, полученную через `systeminfo branch`. |
| [GetKernel](#method-GetKernel) | Возвращает версию ядра, полученную через `systeminfo kernel`. |
| [GetOperationSystemName](#method-GetOperationSystemName) | Возвращает человекочитаемое имя ОС, полученное через `systeminfo os-name`. |
| [GetLicense](#method-GetLicense) | Возвращает текст лицензии с учётом локали и редакции, полученный через `systeminfo os-license`. |
| [GetReleaseNotes](#method-GetReleaseNotes) | Возвращает релиз-ноты, сформированные `systeminfo release-notes`. |
| [GetFinalNotes](#method-GetFinalNotes) | Возвращает финальные заметки инсталлятора, сформированные `systeminfo final-notes`. |
| [GetArch](#method-GetArch) | Возвращает архитектуру системы, полученную через `systeminfo arch`. |
| [GetMonitor](#method-GetMonitor) | Перечисляет подключённые мониторы и разрешения, полученные через `systeminfo monitor`. |
| [GetMotherboard](#method-GetMotherboard) | Возвращает производителя, модель и версию системной платы, полученные через `systeminfo motherboard`. |
| [GetLocale](#method-GetLocale) | Возвращает системную локаль, полученную через `systeminfo locale`. |
| [ListDesktopEnvironments](#method-ListDesktopEnvironments) | Перечисляет доступные сеансы рабочего стола, полученные через `systeminfo list-desktop-environments`. |


## Методы

### **GetAll**() -> ([stdout_bytes](#argument-stdout_bytes-of-GetAll) : `ay`, [response](#argument-response-of-GetAll) : `i`)<a id="method-GetAll"></a>

Собирает сводные сведения о системе в формате ключ/значение.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-GetAll"></a>

Сырые данные, выводимые помощником `systeminfo get-all`.

Строки `KEY="value"` в порядке: HOSTNAME, OS_NAME, BRANCH, KERNEL, CPU, ARCH, GPU, MEMORY, DRIVE, MOTHERBOARD, MONITOR.
##### **response** : `i` <a id="argument-response-of-GetAll"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **GetCPU**() -> ([stdout_strings](#argument-stdout_strings-of-GetCPU) : `as`, [response](#argument-response-of-GetCPU) : `i`)<a id="method-GetCPU"></a>

Возвращает модель CPU, число логических ядер и частоту, полученные через `systeminfo cpu`.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetCPU"></a>

Параметры CPU, возвращаемые помощником.

Элементы stdout_strings: модель; число логических ядер; частота в МГц.
##### **response** : `i` <a id="argument-response-of-GetCPU"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **GetHostName**() -> ([stdout_strings](#argument-stdout_strings-of-GetHostName) : `as`, [response](#argument-response-of-GetHostName) : `i`)<a id="method-GetHostName"></a>

Возвращает имя хоста, полученное через `systeminfo host-name`.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetHostName"></a>

Имя хоста из команды `hostname`.
##### **response** : `i` <a id="argument-response-of-GetHostName"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **GetDrive**() -> ([stdout_strings](#argument-stdout_strings-of-GetDrive) : `as`, [response](#argument-response-of-GetDrive) : `i`)<a id="method-GetDrive"></a>

Возвращает суммарный объём накопителей, полученный через `systeminfo drive`.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetDrive"></a>

Суммарный объём накопителей в байтах, вычисленный по `/proc/partitions`.
##### **response** : `i` <a id="argument-response-of-GetDrive"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **GetGPU**() -> ([stdout_strings](#argument-stdout_strings-of-GetGPU) : `as`, [response](#argument-response-of-GetGPU) : `i`)<a id="method-GetGPU"></a>

Перечисляет VGA-адаптеры, сообщаемые `systeminfo gpu`.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetGPU"></a>

Описания адаптеров, полученные из вывода `lspci`.
##### **response** : `i` <a id="argument-response-of-GetGPU"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **GetMemory**() -> ([stdout_strings](#argument-stdout_strings-of-GetMemory) : `as`, [response](#argument-response-of-GetMemory) : `i`)<a id="method-GetMemory"></a>

Сообщает объём RAM, полученный через `systeminfo memory`.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetMemory"></a>

Объём памяти в байтах, вычисленный как `MemTotal * 1024` из `/proc/meminfo`.
##### **response** : `i` <a id="argument-response-of-GetMemory"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **GetBranch**() -> ([stdout_strings](#argument-stdout_strings-of-GetBranch) : `as`, [response](#argument-response-of-GetBranch) : `i`)<a id="method-GetBranch"></a>

Возвращает ветку дистрибутива, полученную через `systeminfo branch`.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetBranch"></a>

Значение rpm-макроса `%_priority_distbranch`.
##### **response** : `i` <a id="argument-response-of-GetBranch"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **GetKernel**() -> ([stdout_strings](#argument-stdout_strings-of-GetKernel) : `as`, [response](#argument-response-of-GetKernel) : `i`)<a id="method-GetKernel"></a>

Возвращает версию ядра, полученную через `systeminfo kernel`.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetKernel"></a>

Строка выпуска ядра из `/proc/sys/kernel/osrelease`.
##### **response** : `i` <a id="argument-response-of-GetKernel"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **GetOperationSystemName**() -> ([stdout_strings](#argument-stdout_strings-of-GetOperationSystemName) : `as`, [response](#argument-response-of-GetOperationSystemName) : `i`)<a id="method-GetOperationSystemName"></a>

Возвращает человекочитаемое имя операционной системы, полученное через `systeminfo os-name`.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetOperationSystemName"></a>

Значение `PRETTY_NAME` из `/etc/os-release`.
##### **response** : `i` <a id="argument-response-of-GetOperationSystemName"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **GetLicense**() -> ([stdout_bytes](#argument-stdout_bytes-of-GetLicense) : `ay`, [response](#argument-response-of-GetLicense) : `i`)<a id="method-GetLicense"></a>

Возвращает текст лицензии с учётом локали и редакции, полученный через `systeminfo os-license`.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-GetLicense"></a>

Содержимое лицензии, выбранное помощником.

Порядок поиска: `/usr/lib/alterator/backends/edition` (если установлен `alterator-backend-edition-utils`), `license.<lang>.html` в `/usr/share/alt-notes`, `/usr/share/alt-license`, `/var/lib/install3/licenses`; запасной файл — `license.all.html`. Перед запуском LC_ALL очищается.
##### **response** : `i` <a id="argument-response-of-GetLicense"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **GetReleaseNotes**() -> ([stdout_bytes](#argument-stdout_bytes-of-GetReleaseNotes) : `ay`, [response](#argument-response-of-GetReleaseNotes) : `i`)<a id="method-GetReleaseNotes"></a>

Возвращает релиз-ноты, сформированные `systeminfo release-notes`.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-GetReleaseNotes"></a>

Текст релиз-нот, возвращаемый помощником (HTML или текст).

Перед запуском LC_ALL очищается.
##### **response** : `i` <a id="argument-response-of-GetReleaseNotes"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **GetFinalNotes**() -> ([stdout_bytes](#argument-stdout_bytes-of-GetFinalNotes) : `ay`, [response](#argument-response-of-GetFinalNotes) : `i`)<a id="method-GetFinalNotes"></a>

Возвращает финальные заметки инсталлятора, сформированные `systeminfo final-notes`.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-GetFinalNotes"></a>

Текст финальных заметок, формируемый помощником.

Перед запуском LC_ALL очищается.
##### **response** : `i` <a id="argument-response-of-GetFinalNotes"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **GetArch**() -> ([stdout_strings](#argument-stdout_strings-of-GetArch) : `as`, [response](#argument-response-of-GetArch) : `i`)<a id="method-GetArch"></a>

Возвращает архитектуру системы, полученную через `systeminfo arch`.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetArch"></a>

Строка архитектуры из `/proc/sys/kernel/arch`.
##### **response** : `i` <a id="argument-response-of-GetArch"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **GetMonitor**() -> ([stdout_strings](#argument-stdout_strings-of-GetMonitor) : `as`, [response](#argument-response-of-GetMonitor) : `i`)<a id="method-GetMonitor"></a>

Перечисляет подключённые мониторы и разрешения, полученные через `systeminfo monitor`.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetMonitor"></a>

Списки коннекторов и разрешений из `/sys/class/drm/*/modes`.

Элементы формата `<устройство>-<коннектор> <ширина>x<высота>`.
##### **response** : `i` <a id="argument-response-of-GetMonitor"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **GetMotherboard**() -> ([stdout_strings](#argument-stdout_strings-of-GetMotherboard) : `as`, [response](#argument-response-of-GetMotherboard) : `i`)<a id="method-GetMotherboard"></a>

Возвращает производителя, модель и версию системной платы, полученные через `systeminfo motherboard`.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetMotherboard"></a>

Данные платы из `/sys/devices/virtual/dmi/id/board_vendor`, `board_name`, `board_version`.

Элементы stdout_strings: производитель; модель; версия.
##### **response** : `i` <a id="argument-response-of-GetMotherboard"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **GetLocale**() -> ([stdout_strings](#argument-stdout_strings-of-GetLocale) : `as`, [response](#argument-response-of-GetLocale) : `i`)<a id="method-GetLocale"></a>

Возвращает системную локаль, полученную через `systeminfo locale`.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetLocale"></a>

Значение локали из `/etc/locale.conf`, например `ru_RU.UTF-8`.
##### **response** : `i` <a id="argument-response-of-GetLocale"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
### **ListDesktopEnvironments**() -> ([stdout_strings](#argument-stdout_strings-of-ListDesktopEnvironments) : `as`, [response](#argument-response-of-ListDesktopEnvironments) : `i`)<a id="method-ListDesktopEnvironments"></a>

Перечисляет доступные сеансы рабочего стола, полученные через `systeminfo list-desktop-environments`.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-ListDesktopEnvironments"></a>

Список сеансов, собранный по `.desktop`-файлам (plasma, gnome, mate, cinnamon, xfce).

Возможные записи stdout_strings: `CINNAMON`, `GNOME`, `KDE5`, `KDE6`, `MATE`, `XFCE`.
##### **response** : `i` <a id="argument-response-of-ListDesktopEnvironments"></a>

Код завершения помощника.

0 — успех, != 0 — ошибка.
