## Общие сведения

Интерфейс org.altlinux.alterator.systeminfo1 предоставляет доступ к сведениям о системе через скрипт `/usr/lib/alterator/backends/systeminfo`. Методы возвращают код response запущенной команды (0 — успех, != 0 — ошибка).

## Info

- Назначение: публикует описание объекта systeminfo из `/usr/share/alterator/objects/systeminfo.object`.
- Параметры: входных нет; возвращает stdout_bytes и response.
- Ожидаемое поведение (пример): stdout_bytes содержит TOML-описание, response = 0.

## GetAll

- Назначение: вызывает `/usr/lib/alterator/backends/systeminfo --all` для вывода агрегированного набора переменных HOSTNAME/OS_NAME/BRANCH и др.
- Параметры: не принимает аргументов; возвращает stdout_bytes и response.
- Ожидаемое поведение (пример): stdout_bytes содержит набор строк `KEY="value"` для хоста, версии ОС, ядра, CPU, архитектуры, GPU, памяти, накопителей, платы и мониторов; response = 0.

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
- Ожидаемое поведение (пример): stdout_strings содержит три строки (название CPU, количество ядер, частоту в МГц); response = 0.

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
- Ожидаемое поведение (пример): stdout_strings включает сведения о производителе и модели системной платы, response = 0.

## GetLocale

- Назначение: запускает `systeminfo locale`, который считывает `LANG` из `/etc/locale.conf`.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит строку локали в формате `ru_RU.UTF-8`, response = 0.

## ListDesktopEnvironments

- Назначение: выполняет `systeminfo list-desktop-environments`, перебирая десктопные `.desktop` файлы (plasma, gnome, mate, cinnamon, xfce).
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит названия установленных окружений (GNOME, KDE, XFCE и т. п.) по одному в строке, response = 0.
