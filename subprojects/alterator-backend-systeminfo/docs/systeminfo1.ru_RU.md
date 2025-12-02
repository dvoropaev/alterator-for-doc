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

Формат вывода: строки `KEY="value"` в порядке HOSTNAME, OS_NAME, BRANCH, KERNEL, CPU, ARCH, GPU, MEMORY, DRIVE, MOTHERBOARD, MONITOR. Код: 0 — успех, != 0 — ошибка.
#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-GetAll"></a>

##### **response** : `i` <a id="argument-response-of-GetAll"></a>

### **GetCPU**() -> ([stdout_strings](#argument-stdout_strings-of-GetCPU) : `as`, [response](#argument-response-of-GetCPU) : `i`)<a id="method-GetCPU"></a>

Возвращает модель CPU, число логических ядер и частоту, полученные через `systeminfo cpu`.

stdout_strings[0] — модель; stdout_strings[1] — количество логических ядер; stdout_strings[2] — частота в МГц. Код: 0 — успех, != 0 — ошибка.
#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetCPU"></a>

##### **response** : `i` <a id="argument-response-of-GetCPU"></a>

### **GetHostName**() -> ([stdout_strings](#argument-stdout_strings-of-GetHostName) : `as`, [response](#argument-response-of-GetHostName) : `i`)<a id="method-GetHostName"></a>

Возвращает имя хоста, полученное через `systeminfo host-name`.

stdout_strings содержит одно имя хоста, выводимое командой `hostname`. Код: 0 — успех, != 0 — ошибка.
#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetHostName"></a>

##### **response** : `i` <a id="argument-response-of-GetHostName"></a>

### **GetDrive**() -> ([stdout_strings](#argument-stdout_strings-of-GetDrive) : `as`, [response](#argument-response-of-GetDrive) : `i`)<a id="method-GetDrive"></a>

Возвращает суммарный объём накопителей, полученный через `systeminfo drive`.

Суммирует размеры разделов из `/proc/partitions`; stdout_strings содержит объём в байтах. Код: 0 — успех, != 0 — ошибка.
#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetDrive"></a>

##### **response** : `i` <a id="argument-response-of-GetDrive"></a>

### **GetGPU**() -> ([stdout_strings](#argument-stdout_strings-of-GetGPU) : `as`, [response](#argument-response-of-GetGPU) : `i`)<a id="method-GetGPU"></a>

Перечисляет VGA-адаптеры, сообщаемые `systeminfo gpu`.

Разбирает вывод `lspci` для устройств VGA; stdout_strings содержит описания адаптеров. Код: 0 — успех, != 0 — ошибка.
#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetGPU"></a>

##### **response** : `i` <a id="argument-response-of-GetGPU"></a>

### **GetMemory**() -> ([stdout_strings](#argument-stdout_strings-of-GetMemory) : `as`, [response](#argument-response-of-GetMemory) : `i`)<a id="method-GetMemory"></a>

Сообщает объём RAM, полученный через `systeminfo memory`.

Вычисляет `MemTotal * 1024` из `/proc/meminfo`; stdout_strings возвращает объём в байтах. Код: 0 — успех, != 0 — ошибка.
#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetMemory"></a>

##### **response** : `i` <a id="argument-response-of-GetMemory"></a>

### **GetBranch**() -> ([stdout_strings](#argument-stdout_strings-of-GetBranch) : `as`, [response](#argument-response-of-GetBranch) : `i`)<a id="method-GetBranch"></a>

Возвращает ветку дистрибутива, полученную через `systeminfo branch`.

Предоставляет значение rpm-макроса `%_priority_distbranch` для служебного использования. Код: 0 — успех, != 0 — ошибка.
#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetBranch"></a>

##### **response** : `i` <a id="argument-response-of-GetBranch"></a>

### **GetKernel**() -> ([stdout_strings](#argument-stdout_strings-of-GetKernel) : `as`, [response](#argument-response-of-GetKernel) : `i`)<a id="method-GetKernel"></a>

Возвращает версию ядра, полученную через `systeminfo kernel`.

Читает `/proc/sys/kernel/osrelease`; stdout_strings содержит строку выпуска ядра. Код: 0 — успех, != 0 — ошибка.
#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetKernel"></a>

##### **response** : `i` <a id="argument-response-of-GetKernel"></a>

### **GetOperationSystemName**() -> ([stdout_strings](#argument-stdout_strings-of-GetOperationSystemName) : `as`, [response](#argument-response-of-GetOperationSystemName) : `i`)<a id="method-GetOperationSystemName"></a>

Возвращает человекочитаемое имя операционной системы, полученное через `systeminfo os-name`.

Считывает `PRETTY_NAME` из `/etc/os-release`; stdout_strings содержит отображаемое имя. Код: 0 — успех, != 0 — ошибка.
#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetOperationSystemName"></a>

##### **response** : `i` <a id="argument-response-of-GetOperationSystemName"></a>

### **GetLicense**() -> ([stdout_bytes](#argument-stdout_bytes-of-GetLicense) : `ay`, [response](#argument-response-of-GetLicense) : `i`)<a id="method-GetLicense"></a>

Возвращает текст лицензии с учётом локали и редакции, полученный через `systeminfo os-license`.

Порядок поиска: `/usr/lib/alterator/backends/edition` (при наличии `alterator-backend-edition-utils`), затем `license.<lang>.html` в `/usr/share/alt-notes`, `/usr/share/alt-license`, `/var/lib/install3/licenses`; запасной файл — `license.all.html`. Перед запуском LC_ALL очищается. Код: 0 — успех, != 0 — ошибка.
#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-GetLicense"></a>

##### **response** : `i` <a id="argument-response-of-GetLicense"></a>

### **GetReleaseNotes**() -> ([stdout_bytes](#argument-stdout_bytes-of-GetReleaseNotes) : `ay`, [response](#argument-response-of-GetReleaseNotes) : `i`)<a id="method-GetReleaseNotes"></a>

Возвращает релиз-ноты, сформированные `systeminfo release-notes`.

Выводит HTML или текстовые заметки; LC_ALL очищается. Код: 0 — успех, != 0 — ошибка.
#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-GetReleaseNotes"></a>

##### **response** : `i` <a id="argument-response-of-GetReleaseNotes"></a>

### **GetFinalNotes**() -> ([stdout_bytes](#argument-stdout_bytes-of-GetFinalNotes) : `ay`, [response](#argument-response-of-GetFinalNotes) : `i`)<a id="method-GetFinalNotes"></a>

Возвращает финальные заметки инсталлятора, сформированные `systeminfo final-notes`.

Предоставляет заключительные подсказки для установщика; LC_ALL очищается. Код: 0 — успех, != 0 — ошибка.
#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-GetFinalNotes"></a>

##### **response** : `i` <a id="argument-response-of-GetFinalNotes"></a>

### **GetArch**() -> ([stdout_strings](#argument-stdout_strings-of-GetArch) : `as`, [response](#argument-response-of-GetArch) : `i`)<a id="method-GetArch"></a>

Возвращает архитектуру системы, полученную через `systeminfo arch`.

Читает `/proc/sys/kernel/arch`; stdout_strings содержит строку архитектуры, например x86_64. Код: 0 — успех, != 0 — ошибка.
#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetArch"></a>

##### **response** : `i` <a id="argument-response-of-GetArch"></a>

### **GetMonitor**() -> ([stdout_strings](#argument-stdout_strings-of-GetMonitor) : `as`, [response](#argument-response-of-GetMonitor) : `i`)<a id="method-GetMonitor"></a>

Перечисляет подключённые мониторы и разрешения, полученные через `systeminfo monitor`.

Читает `/sys/class/drm/*/modes` и сопоставляет с коннекторами; элементы stdout_strings выглядят как `card0-HDMI-A-1 1920x1080`. Код: 0 — успех, != 0 — ошибка.
#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetMonitor"></a>

##### **response** : `i` <a id="argument-response-of-GetMonitor"></a>

### **GetMotherboard**() -> ([stdout_strings](#argument-stdout_strings-of-GetMotherboard) : `as`, [response](#argument-response-of-GetMotherboard) : `i`)<a id="method-GetMotherboard"></a>

Возвращает производителя, модель и версию системной платы, полученные через `systeminfo motherboard`.

Объединяет `/sys/devices/virtual/dmi/id/board_vendor`, `board_name`, `board_version`. stdout_strings[0] — производитель; stdout_strings[1] — модель; stdout_strings[2] — версия. Код: 0 — успех, != 0 — ошибка.
#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetMotherboard"></a>

##### **response** : `i` <a id="argument-response-of-GetMotherboard"></a>

### **GetLocale**() -> ([stdout_strings](#argument-stdout_strings-of-GetLocale) : `as`, [response](#argument-response-of-GetLocale) : `i`)<a id="method-GetLocale"></a>

Возвращает системную локаль, полученную через `systeminfo locale`.

Считывает `LANG` из `/etc/locale.conf`; stdout_strings содержит локаль, например `ru_RU.UTF-8`. Код: 0 — успех, != 0 — ошибка.
#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-GetLocale"></a>

##### **response** : `i` <a id="argument-response-of-GetLocale"></a>

### **ListDesktopEnvironments**() -> ([stdout_strings](#argument-stdout_strings-of-ListDesktopEnvironments) : `as`, [response](#argument-response-of-ListDesktopEnvironments) : `i`)<a id="method-ListDesktopEnvironments"></a>

Перечисляет доступные сеансы рабочего стола, полученные через `systeminfo list-desktop-environments`.

Перебирает `.desktop`-файлы для plasma, gnome, mate, cinnamon, xfce; stdout_strings может включать записи `CINNAMON`, `GNOME`, `KDE<index>`, `MATE`, `XFCE`. Код: 0 — успех, != 0 — ошибка.
#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-ListDesktopEnvironments"></a>

##### **response** : `i` <a id="argument-response-of-ListDesktopEnvironments"></a>
