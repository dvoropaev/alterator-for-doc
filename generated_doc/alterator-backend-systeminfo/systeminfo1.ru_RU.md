[English](systeminfo1.md) | [Русский](systeminfo1.ru_RU.md)

## Общие сведения

Интерфейс org.altlinux.alterator.systeminfo1 предоставляет доступ к сведениям о системе через скрипт `/usr/lib/alterator/backends/systeminfo`. Регистрацию на шине выполняет подсистема `alterator-module-executor` менеджера `alterator-manager` по описанию `/usr/share/alterator/backends/systeminfo.backend`. Методы возвращают код response запущенной команды (0 — успех, != 0 — ошибка).

## `Info() -> (stdout_bytes: ay, response: i)`

- Назначение: публикует описание объекта systeminfo из `/usr/share/alterator/objects/systeminfo.object`.
- Параметры: входных нет; возвращает stdout_bytes и response.
- Ожидаемое поведение (пример): stdout_bytes содержит TOML-описание, response = 0.

## `GetAll() -> (stdout_bytes: ay, response: i)`

- Назначение: собрать сводные сведения о системе для передачи клиенту.
- Параметры: не принимает аргументов; возвращает stdout_bytes и response.
- Формат stdout_bytes: строки вида `KEY="value"` с ключами `HOSTNAME`, `OS_NAME`, `BRANCH`, `KERNEL`, `CPU`, `ARCH`, `GPU`, `MEMORY`, `DRIVE`, `MOTHERBOARD`, `MONITOR` в указанном порядке.
- Ожидаемое поведение (пример): response = 0, вывод содержит все перечисленные пары ключ-значение.

## `GetHostName() -> (stdout_strings: as, response: i)`

- Назначение: запускает `systeminfo host-name`, который печатает результат `hostname`.
- Параметры: не принимает аргументов; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит одно значение имени узла, response = 0.

## `GetOperationSystemName() -> (stdout_strings: as, response: i)`

- Назначение: получить имя операционной системы в человекочитаемом виде для показа пользователю; выполняет `systeminfo os-name`, считывая `PRETTY_NAME` из `/etc/os-release`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит человекочитаемое имя дистрибутива, response = 0.

## `GetLicense() -> (stdout_bytes: ay, response: i)`

- Назначение: вывести лицензионный текст с учётом локали и наличия редакции; вызывает `systeminfo os-license`, который сначала обращается к `/usr/lib/alterator/backends/edition` (при наличии `alterator-backend-edition-utils`), затем ищет `license.<язык>.html` в `/usr/share/alt-notes`, `/usr/share/alt-license`, `/var/lib/install3/licenses` и использует `license.all.html` как запасной файл.
- Параметры: входных аргументов нет; возвращает stdout_bytes и response (LC_ALL очищен).
- Ожидаемое поведение (пример): stdout_bytes содержит содержимое лицензии текущей редакции, response = 0; при отсутствии файла команда завершается с ошибкой.

## `GetReleaseNotes() -> (stdout_bytes: ay, response: i)`

- Назначение: выполняет `systeminfo release-notes` для вывода текстов релиз-нот.
- Параметры: аргументов нет; возвращает stdout_bytes и response (LC_ALL очищен).
- Ожидаемое поведение (пример): stdout_bytes содержит HTML/текст заметок выпуска; отсутствие файла приводит к ненулевому response.

## `GetFinalNotes() -> (stdout_bytes: ay, response: i)`

- Назначение: запускает `systeminfo final-notes` для получения финальных подсказок инсталлятора.
- Параметры: аргументов нет; возвращает stdout_bytes и response (LC_ALL очищен).
- Ожидаемое поведение (пример): stdout_bytes содержит текст финальных заметок; при отсутствии данных команда завершится с ошибкой.

## `GetArch() -> (stdout_strings: as, response: i)`

- Назначение: выполняет `systeminfo arch`, считывая архитектуру из `/proc/sys/kernel/arch`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит архитектуру (например, x86_64), response = 0.

## `GetBranch() -> (stdout_strings: as, response: i)`

- Назначение: получить ветку дистрибутива для служебного отображения; вызывает `systeminfo branch`, возвращая значение rpm-макроса `%_priority_distbranch`.
- Параметры: не принимает аргументов; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings включает текущую ветку репозитория, response = 0.

## `GetKernel() -> (stdout_strings: as, response: i)`

- Назначение: запускает `systeminfo kernel`, выводящий `osrelease` из `/proc/sys/kernel/osrelease`.
- Параметры: входных аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит версию ядра, response = 0.

## `GetCPU() -> (stdout_strings: as, response: i)`

- Назначение: выполняет `systeminfo cpu`, который извлекает модель, число логических ядер и частоту из `/proc/cpuinfo` и sysfs.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Формат stdout_strings: `[0]` — название CPU, `[1]` — количество логических ядер, `[2]` — частота в МГц.
- Ожидаемое поведение (пример): response = 0, все три строки присутствуют.

## `GetGPU() -> (stdout_strings: as, response: i)`

- Назначение: запускает `systeminfo gpu`, парся вывод `lspci` по шаблону VGA.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит список адаптеров VGA, response = 0.

## `GetMemory() -> (stdout_strings: as, response: i)`

- Назначение: выполняет `systeminfo memory`, вычисляя объём RAM из `/proc/meminfo` (значение MemTotal * 1024).
- Параметры: входных аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит объём памяти в байтах, response = 0.

## `GetDrive() -> (stdout_strings: as, response: i)`

- Назначение: запускает `systeminfo drive`, который суммирует размеры разделов из `/proc/partitions`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит суммарный объём накопителей в байтах, response = 0.

## `GetMonitor() -> (stdout_strings: as, response: i)`

- Назначение: выполняет `systeminfo monitor`, читая файлы `/sys/class/drm/*/modes` и сопоставляя их разрешениям.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит список строк вида `card0-HDMI-A-1 1920x1080`, response = 0.

## `GetMotherboard() -> (stdout_strings: as, response: i)`

- Назначение: вызывает `systeminfo motherboard`, объединяя содержимое `/sys/devices/virtual/dmi/id/board_{vendor,name,version}`.
- Параметры: входных аргументов нет; возвращает stdout_strings и response.
- Формат stdout_strings: `[0]` — производитель платы, `[1]` — модель, `[2]` — версия.
- Ожидаемое поведение (пример): response = 0, вывод содержит все три строки.

## `GetLocale() -> (stdout_strings: as, response: i)`

- Назначение: запускает `systeminfo locale`, который считывает `LANG` из `/etc/locale.conf`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Формат stdout_strings: значение локали в виде `<язык>_<регион>.<кодировка>` (например, `ru_RU.UTF-8`).
- Ожидаемое поведение (пример): response = 0, строка локали присутствует.

## `ListDesktopEnvironments() -> (stdout_strings: as, response: i)`

- Назначение: выполняет `systeminfo list-desktop-environments`, перебирая десктопные `.desktop` файлы (plasma, gnome, mate, cinnamon, xfce).
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит по одному значению в строке из множества `CINNAMON`, `GNOME`, `KDE<номер>`, `MATE`, `XFCE`; response = 0.
