## Общие сведения

Интерфейс org.altlinux.alterator.systeminfo1 предоставляет доступ к сведениям о системе через скрипт `/usr/lib/alterator/backends/systeminfo`. Регистрацию на шине выполняет подсистема `alterator-module-executor` менеджера `alterator-manager` по описанию `/usr/share/alterator/backends/systeminfo.backend`. Методы возвращают код response запущенной команды (0 — успех, != 0 — ошибка).

## Info

- Назначение: публикует описание объекта systeminfo из `/usr/share/alterator/objects/systeminfo.object`.
- Сигнатура: `Info() -> (stdout_bytes: ay, response: i)`.
- Параметры: входных нет; возвращает stdout_bytes и response.
- Ожидаемое поведение (пример): stdout_bytes содержит TOML-описание, response = 0.

## GetAll

- Назначение: вызывает `/usr/lib/alterator/backends/systeminfo --all` и собирает агрегированный набор характеристик.
- Сигнатура: `GetAll() -> (stdout_bytes: ay, response: i)`.
- Параметры: не принимает аргументов; возвращает stdout_bytes и response.
- Формат stdout_bytes: строки вида `KEY="value"` с ключами `HOSTNAME`, `OS_NAME`, `BRANCH`, `KERNEL`, `CPU`, `ARCH`, `GPU`, `MEMORY`, `DRIVE`, `MOTHERBOARD`, `MONITOR` в указанном порядке.
- Ожидаемое поведение (пример): response = 0, вывод содержит все перечисленные пары ключ-значение.

## GetHostName

- Назначение: запускает `systeminfo host-name`, который печатает результат `hostname`.
- Сигнатура: `GetHostName() -> (stdout_strings: as, response: i)`.
- Параметры: не принимает аргументов; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит одно значение имени узла, response = 0.

## GetOperationSystemName

- Назначение: выполняет `systeminfo os-name`, считывая `PRETTY_NAME` из `/etc/os-release`.
- Сигнатура: `GetOperationSystemName() -> (stdout_strings: as, response: i)`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит человекочитаемое имя дистрибутива, response = 0.

## GetLicense

- Назначение: вызывает `systeminfo os-license`, который через `get_notes_file_path` ищет лицензионный текст и выводит файл.
- Сигнатура: `GetLicense() -> (stdout_bytes: ay, response: i)`.
- Параметры: входных аргументов нет; возвращает stdout_bytes и response (LC_ALL очищен).
- Ожидаемое поведение (пример): stdout_bytes содержит содержимое лицензии текущей редакции, response = 0; при отсутствии файла команда завершается с ошибкой.

## GetReleaseNotes

- Назначение: выполняет `systeminfo release-notes` для вывода текстов релиз-нот.
- Сигнатура: `GetReleaseNotes() -> (stdout_bytes: ay, response: i)`.
- Параметры: аргументов нет; возвращает stdout_bytes и response (LC_ALL очищен).
- Ожидаемое поведение (пример): stdout_bytes содержит HTML/текст заметок выпуска; отсутствие файла приводит к ненулевому response.

## GetFinalNotes

- Назначение: запускает `systeminfo final-notes` для получения финальных подсказок инсталлятора.
- Сигнатура: `GetFinalNotes() -> (stdout_bytes: ay, response: i)`.
- Параметры: аргументов нет; возвращает stdout_bytes и response (LC_ALL очищен).
- Ожидаемое поведение (пример): stdout_bytes содержит текст финальных заметок; при отсутствии данных команда завершится с ошибкой.

## GetArch

- Назначение: выполняет `systeminfo arch`, считывая архитектуру из `/proc/sys/kernel/arch`.
- Сигнатура: `GetArch() -> (stdout_strings: as, response: i)`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит архитектуру (например, x86_64), response = 0.

## GetBranch

- Назначение: вызывает `systeminfo branch`, возвращая значение `%_priority_distbranch` из rpm.
- Сигнатура: `GetBranch() -> (stdout_strings: as, response: i)`.
- Параметры: не принимает аргументов; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings включает текущую ветку репозитория, response = 0.

## GetKernel

- Назначение: запускает `systeminfo kernel`, выводящий `osrelease` из `/proc/sys/kernel/osrelease`.
- Сигнатура: `GetKernel() -> (stdout_strings: as, response: i)`.
- Параметры: входных аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит версию ядра, response = 0.

## GetCPU

- Назначение: выполняет `systeminfo cpu`, который извлекает модель, число логических ядер и частоту из `/proc/cpuinfo` и sysfs.
- Сигнатура: `GetCPU() -> (stdout_strings: as, response: i)`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Формат stdout_strings: `[0]` — название CPU, `[1]` — количество логических ядер, `[2]` — частота в МГц.
- Ожидаемое поведение (пример): response = 0, все три строки присутствуют.

## GetGPU

- Назначение: запускает `systeminfo gpu`, парся вывод `lspci` по шаблону VGA.
- Сигнатура: `GetGPU() -> (stdout_strings: as, response: i)`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит список адаптеров VGA, response = 0.

## GetMemory

- Назначение: выполняет `systeminfo memory`, вычисляя объём RAM из `/proc/meminfo` (значение MemTotal * 1024).
- Сигнатура: `GetMemory() -> (stdout_strings: as, response: i)`.
- Параметры: входных аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит объём памяти в байтах, response = 0.

## GetDrive

- Назначение: запускает `systeminfo drive`, который суммирует размеры разделов из `/proc/partitions`.
- Сигнатура: `GetDrive() -> (stdout_strings: as, response: i)`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит суммарный объём накопителей в байтах, response = 0.

## GetMonitor

- Назначение: выполняет `systeminfo monitor`, читая файлы `/sys/class/drm/*/modes` и сопоставляя их разрешениям.
- Сигнатура: `GetMonitor() -> (stdout_strings: as, response: i)`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит список строк вида `card0-HDMI-A-1 1920x1080`, response = 0.

## GetMotherboard

- Назначение: вызывает `systeminfo motherboard`, объединяя содержимое `/sys/devices/virtual/dmi/id/board_{vendor,name,version}`.
- Сигнатура: `GetMotherboard() -> (stdout_strings: as, response: i)`.
- Параметры: входных аргументов нет; возвращает stdout_strings и response.
- Формат stdout_strings: `[0]` — производитель платы, `[1]` — модель, `[2]` — версия.
- Ожидаемое поведение (пример): response = 0, вывод содержит все три строки.

## GetLocale

- Назначение: запускает `systeminfo locale`, который считывает `LANG` из `/etc/locale.conf`.
- Сигнатура: `GetLocale() -> (stdout_strings: as, response: i)`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Формат stdout_strings: значение локали в виде `<язык>_<регион>.<кодировка>` (например, `ru_RU.UTF-8`).
- Ожидаемое поведение (пример): response = 0, строка локали присутствует.

## ListDesktopEnvironments

- Назначение: выполняет `systeminfo list-desktop-environments`, перебирая десктопные `.desktop` файлы (plasma, gnome, mate, cinnamon, xfce).
- Сигнатура: `ListDesktopEnvironments() -> (stdout_strings: as, response: i)`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит по одному значению в строке из множества `CINNAMON`, `GNOME`, `KDE<номер>`, `MATE`, `XFCE`; response = 0.
