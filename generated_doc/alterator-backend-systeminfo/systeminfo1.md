## Общие сведения

Интерфейс org.altlinux.alterator.systeminfo1 предоставляет доступ к сведениям о системе через скрипт `/usr/lib/alterator/backends/systeminfo`. Регистрацию на шине выполняет `alterator-manager` (подсистема `alterator-module-executor`) по описанию `/usr/share/alterator/backends/systeminfo.backend`. Методы возвращают код response запущенной команды (0 — успех, != 0 — ошибка).

## Info

- Назначение: публикует описание объекта systeminfo из `/usr/share/alterator/objects/systeminfo.object`.
- Параметры: входных нет; возвращает stdout_bytes и response.
- Ожидаемое поведение (пример): stdout_bytes содержит TOML-описание, response = 0.

## GetAll

- Назначение: вызывает `/usr/lib/alterator/backends/systeminfo --all` и собирает агрегированный набор характеристик.
- Параметры: не принимает аргументов; возвращает stdout_bytes и response.
- Формат stdout_bytes: строки вида `KEY="value"` с ключами `HOSTNAME`, `OS_NAME`, `BRANCH`, `KERNEL`, `CPU`, `ARCH`, `GPU`, `MEMORY`, `DRIVE`, `MOTHERBOARD`, `MONITOR` в указанном порядке.
- Ожидаемое поведение (пример): response = 0, вывод содержит все перечисленные пары ключ-значение.

## GetHostName

- Назначение: запускает `systeminfo host-name`, который печатает результат `hostname`.
- Параметры: не принимает аргументов; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит одно значение имени узла, response = 0.

## GetOperationSystemName

- Назначение: выполняет `systeminfo os-name`, считывая `PRETTY_NAME` из `/etc/os-release`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит человекочитаемое имя дистрибутива, response = 0.

## GetLicense

- Назначение: вызывает `systeminfo os-license`, который через `get_notes_file_path` ищет лицензионный текст и выводит файл.
- Параметры: входных аргументов нет; возвращает stdout_bytes и response (LC_ALL очищен).
- Ожидаемое поведение (пример): stdout_bytes содержит содержимое лицензии текущей редакции, response = 0; при отсутствии файла команда завершается с ошибкой.

## GetReleaseNotes

- Назначение: выполняет `systeminfo release-notes` для вывода текстов релиз-нот.
- Параметры: аргументов нет; возвращает stdout_bytes и response (LC_ALL очищен).
- Ожидаемое поведение (пример): stdout_bytes содержит HTML/текст заметок выпуска; отсутствие файла приводит к ненулевому response.

## GetFinalNotes

- Назначение: запускает `systeminfo final-notes` для получения финальных подсказок инсталлятора.
- Параметры: аргументов нет; возвращает stdout_bytes и response (LC_ALL очищен).
- Ожидаемое поведение (пример): stdout_bytes содержит текст финальных заметок; при отсутствии данных команда завершится с ошибкой.

## GetArch

- Назначение: выполняет `systeminfo arch`, считывая архитектуру из `/proc/sys/kernel/arch`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит архитектуру (например, x86_64), response = 0.

## GetBranch

- Назначение: вызывает `systeminfo branch`, возвращая значение `%_priority_distbranch` из rpm.
- Параметры: не принимает аргументов; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings включает текущую ветку репозитория, response = 0.

## GetKernel

- Назначение: запускает `systeminfo kernel`, выводящий `osrelease` из `/proc/sys/kernel/osrelease`.
- Параметры: входных аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит версию ядра, response = 0.

## GetCPU

- Назначение: выполняет `systeminfo cpu`, который извлекает модель, число логических ядер и частоту из `/proc/cpuinfo` и sysfs.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Формат stdout_strings: `[0]` — название CPU, `[1]` — количество логических ядер, `[2]` — частота в МГц.
- Ожидаемое поведение (пример): response = 0, все три строки присутствуют.

## GetGPU

- Назначение: запускает `systeminfo gpu`, парся вывод `lspci` по шаблону VGA.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит список адаптеров VGA, response = 0.

## GetMemory

- Назначение: выполняет `systeminfo memory`, вычисляя объём RAM из `/proc/meminfo` (значение MemTotal * 1024).
- Параметры: входных аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит объём памяти в байтах, response = 0.

## GetDrive

- Назначение: запускает `systeminfo drive`, который суммирует размеры разделов из `/proc/partitions`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит суммарный объём накопителей в байтах, response = 0.

## GetMonitor

- Назначение: выполняет `systeminfo monitor`, читая файлы `/sys/class/drm/*/modes` и сопоставляя их разрешениям.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит список строк вида `card0-HDMI-A-1 1920x1080`, response = 0.

## GetMotherboard

- Назначение: вызывает `systeminfo motherboard`, объединяя содержимое `/sys/devices/virtual/dmi/id/board_{vendor,name,version}`.
- Параметры: входных аргументов нет; возвращает stdout_strings и response.
- Формат stdout_strings: `[0]` — производитель платы, `[1]` — модель, `[2]` — версия.
- Ожидаемое поведение (пример): response = 0, вывод содержит все три строки.

## GetLocale

- Назначение: запускает `systeminfo locale`, который считывает `LANG` из `/etc/locale.conf`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Формат stdout_strings: значение локали в виде `<язык>_<регион>.<кодировка>` (например, `ru_RU.UTF-8`).
- Ожидаемое поведение (пример): response = 0, строка локали присутствует.

## ListDesktopEnvironments

- Назначение: выполняет `systeminfo list-desktop-environments`, перебирая десктопные `.desktop` файлы (plasma, gnome, mate, cinnamon, xfce).
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит по одному значению в строке из множества `CINNAMON`, `GNOME`, `KDE<номер>`, `MATE`, `XFCE`; response = 0.
