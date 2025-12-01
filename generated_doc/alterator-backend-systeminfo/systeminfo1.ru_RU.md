[English](systeminfo1.md) | [Русский](systeminfo1.ru_RU.md)

## Общие сведения

Интерфейс org.altlinux.alterator.systeminfo1 предоставляет доступ к сведениям о системе через скрипт `/usr/lib/alterator/backends/systeminfo`. Регистрацию на шине выполняет подсистема `alterator-module-executor` менеджера `alterator-manager` по описанию `/usr/share/alterator/backends/systeminfo.backend`. Методы возвращают код response запущенной команды (0 — успех, != 0 — ошибка).

## Info — `Info() -> (stdout_bytes: ay, response: i)`

- Назначение: публикует описание объекта systeminfo из `/usr/share/alterator/objects/systeminfo.object`.
- Параметры: входных нет; возвращает stdout_bytes и response.
- Ожидаемое поведение (пример): stdout_bytes содержит TOML-описание, response = 0.

## GetAll — `GetAll() -> (stdout_bytes: ay, response: i)`

- Назначение: получить полный набор сведений о машине и установленной системе в одном вызове.
- Параметры: не принимает аргументов; возвращает stdout_bytes и response.
- Формат stdout_bytes: строки вида `KEY="value"` с ключами `HOSTNAME`, `OS_NAME`, `BRANCH`, `KERNEL`, `CPU`, `ARCH`, `GPU`, `MEMORY`, `DRIVE`, `MOTHERBOARD`, `MONITOR` в указанном порядке.
- Ожидаемое поведение (пример): response = 0, вывод содержит все перечисленные пары ключ-значение.

## GetHostName — `GetHostName() -> (stdout_strings: as, response: i)`

- Назначение: получить имя узла, используемое в D-Bus-ответах и других командах.
- Параметры: не принимает аргументов; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит одно значение имени узла, response = 0.

## GetOperationSystemName — `GetOperationSystemName() -> (stdout_strings: as, response: i)`

- Назначение: получить имя операционной системы в формате, удобном для представления пользователю; вызывает `systeminfo os-name`, считывающий `PRETTY_NAME` из `/etc/os-release`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит человекочитаемое имя дистрибутива, response = 0.

## GetLicense — `GetLicense() -> (stdout_bytes: ay, response: i)`

- Назначение: вернуть текст лицензии с учётом редакции и локали.
- Параметры: входных аргументов нет; возвращает stdout_bytes и response (LC_ALL очищен).
- Логика поиска: при наличии пакета `alterator-backend-edition-utils` вызывается `/usr/lib/alterator/backends/edition`, который пытается найти файл в `/usr/share/alt-notes`; если файл не найден или редакция отсутствует, последовательно просматриваются `/usr/share/alt-notes`, `/usr/share/alt-license`, `/var/lib/install3/licenses` по локали, затем `license.all.html`.
- Ожидаемое поведение (пример): stdout_bytes содержит содержимое лицензии, response = 0; при отсутствии файла команда завершается с ошибкой.

## GetReleaseNotes — `GetReleaseNotes() -> (stdout_bytes: ay, response: i)`

- Назначение: вернуть текст заметок релиза с учётом локали и наличия редакции.
- Параметры: аргументов нет; возвращает stdout_bytes и response (LC_ALL очищен).
- Логика поиска: аналогична `GetLicense` — сначала используется информация редакции из `/usr/lib/alterator/backends/edition`, затем поиск по каталогам `/usr/share/alt-notes`, `/usr/share/alt-license`, `/var/lib/install3/licenses` с подстановкой локали или файла `license.all.html`.
- Ожидаемое поведение (пример): stdout_bytes содержит HTML/текст заметок выпуска; отсутствие подходящего файла приводит к ненулевому response.

## GetFinalNotes — `GetFinalNotes() -> (stdout_bytes: ay, response: i)`

- Назначение: предоставить итоговые указания инсталлятора для локали пользователя.
- Параметры: аргументов нет; возвращает stdout_bytes и response (LC_ALL очищен).
- Логика поиска: совпадает с `GetLicense` и `GetReleaseNotes`, включая обращение к `/usr/lib/alterator/backends/edition` при наличии редакции и последующий обход каталогов с учётом локали.
- Ожидаемое поведение (пример): stdout_bytes содержит текст финальных заметок; при отсутствии данных команда завершится с ошибкой.

## GetArch — `GetArch() -> (stdout_strings: as, response: i)`

- Назначение: получить архитектуру машины для отображения в сводных сведениях.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит архитектуру (например, x86_64), response = 0.

## GetBranch — `GetBranch() -> (stdout_strings: as, response: i)`

- Назначение: возвращает значение rpm-макроса `%_priority_distbranch`.
- Параметры: не принимает аргументов; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings включает текущую ветку репозитория, response = 0.

## GetKernel — `GetKernel() -> (stdout_strings: as, response: i)`

- Назначение: запускает `systeminfo kernel`, выводящий `osrelease` из `/proc/sys/kernel/osrelease`.
- Параметры: входных аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит версию ядра, response = 0.

## GetCPU — `GetCPU() -> (stdout_strings: as, response: i)`

- Назначение: выполняет `systeminfo cpu`, который извлекает модель, число логических ядер и частоту из `/proc/cpuinfo` и sysfs.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Формат stdout_strings: `[0]` — название CPU, `[1]` — количество логических ядер, `[2]` — частота в МГц.
- Ожидаемое поведение (пример): response = 0, все три строки присутствуют.

## GetGPU — `GetGPU() -> (stdout_strings: as, response: i)`

- Назначение: запускает `systeminfo gpu`, парся вывод `lspci` по шаблону VGA.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит список адаптеров VGA, response = 0.

## GetMemory — `GetMemory() -> (stdout_strings: as, response: i)`

- Назначение: выполняет `systeminfo memory`, вычисляя объём RAM из `/proc/meminfo` (значение MemTotal * 1024).
- Параметры: входных аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит объём памяти в байтах, response = 0.

## GetDrive — `GetDrive() -> (stdout_strings: as, response: i)`

- Назначение: запускает `systeminfo drive`, который суммирует размеры разделов из `/proc/partitions`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит суммарный объём накопителей в байтах, response = 0.

## GetMonitor — `GetMonitor() -> (stdout_strings: as, response: i)`

- Назначение: выполняет `systeminfo monitor`, читая файлы `/sys/class/drm/*/modes` и сопоставляя их разрешениям.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит список строк вида `card0-HDMI-A-1 1920x1080`, response = 0.

## GetMotherboard — `GetMotherboard() -> (stdout_strings: as, response: i)`

- Назначение: вызывает `systeminfo motherboard`, объединяя содержимое `/sys/devices/virtual/dmi/id/board_{vendor,name,version}`.
- Параметры: входных аргументов нет; возвращает stdout_strings и response.
- Формат stdout_strings: `[0]` — производитель платы, `[1]` — модель, `[2]` — версия.
- Ожидаемое поведение (пример): response = 0, вывод содержит все три строки.

## GetLocale — `GetLocale() -> (stdout_strings: as, response: i)`

- Назначение: запускает `systeminfo locale`, который считывает `LANG` из `/etc/locale.conf`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Формат stdout_strings: значение локали в виде `<язык>_<регион>.<кодировка>` (например, `ru_RU.UTF-8`).
- Ожидаемое поведение (пример): response = 0, строка локали присутствует.

## ListDesktopEnvironments — `ListDesktopEnvironments() -> (stdout_strings: as, response: i)`

- Назначение: выполняет `systeminfo list-desktop-environments`, перебирая десктопные `.desktop` файлы (plasma, gnome, mate, cinnamon, xfce).
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит по одному значению в строке из множества `CINNAMON`, `GNOME`, `KDE<номер>`, `MATE`, `XFCE`; response = 0.
