**alterator-backend-systeminfo**

[Русский](README.ru_RU.md)

Предназначен для предоставления клиентам Alterator сведений о системе через интерфейс D-Bus.

---

# Общие сведения
| Компонент | Расположение | Назначение |
| --------- | ------------ | ---------- |
| Скрипт `systeminfo` | `/usr/lib/alterator/backends/systeminfo` | Выполняет команды выборки сведений об установленной системе. |
| Файл `systeminfo.backend` | `/usr/share/alterator/backends/systeminfo.backend` | Описывает backend с методами интерфейса `org.altlinux.alterator.systeminfo1`. |
| Файл `systeminfo.object` | `/usr/share/alterator/objects/systeminfo.object` | Регистрирует объект Alterator «О системе» для клиентов. |
| Скрипт `systeminfo.d/notes` | `/usr/lib/alterator/backends/systeminfo.d/notes` | Предоставляет вспомогательные функции поиска лицензий, заметок релиза и записей информации о дистрибутиве. |

# Возможности
- Получение текстовых характеристик системы: имя хоста, ветка репозитория, версия ядра, локаль.
- Выдача аппаратных сведений: CPU, GPU, объём памяти, накопители, мониторы, материнская плата.
- Предоставление ссылок на лицензию, заметки релиза и финальные инструкции установки с учётом локали.
- Формирование агрегированного вывода по команде `--all` для передачи в `GetAll`.
- Список установленных окружений рабочего стола по команде `list-desktop-environments`.

# Интеграция с другими компонентами
- Backend использует интерфейс `org.altlinux.alterator.systeminfo1` и отдаёт данные клиентам через D-Bus.
- Интерфейс регистрируется модулем alterator-manager (`alterator-module-executor`) по описанию `/usr/share/alterator/backends/systeminfo.backend`.
- Метод `GetLicense` и связанные команды делегируют поиск файлов модулю `systeminfo.d/notes`, который при наличии вызывает `/usr/lib/alterator/backends/edition` для получения данных текущей редакции.
- Файл `systeminfo.object` включает объект категории `X-Alterator-System`, что позволяет отображать модуль в `alterator-explorer`.

# Команды `systeminfo`
| Команда | Выходные данные | Источники |
| ------- | ---------------- | --------- |
| `host-name`, `os-name`, `branch`, `kernel` | Текстовые строки с общими сведениями системы. | `/etc/os-release`, `%_priority_distbranch`, `/proc/sys/kernel/osrelease`. |
| `os-license`, `release-notes`, `final-notes` | Содержимое HTML-файлов лицензии и заметок. | Поиск через `systeminfo.d/notes` (в том числе в `/usr/share/alt-notes`, `/usr/share/distro-licenses`). |
| `locale`, `list-desktop-environments` | Текущая локаль, перечень окружений рабочего стола. | Чтение `/etc/locale.conf`, обход `.desktop` файлов окружений. |
| `cpu`, `gpu`, `memory`, `drive`, `motherboard`, `monitor` | Аппаратные сведения, обработанные из `/proc` и `/sys`. | Чтение системных файлов и вывод `lspci`/`lscpu`. |

Важно: все команды выполняются с `set -o pipefail`, ошибки приводят к коду возврата `!= 0` и диагностике в stderr.

# Документация по интерфейсам
- `systeminfo1` — предоставляет сведения об установленной системе, включая характеристики оборудования и служебные заметки. См. [systeminfo1.md](./systeminfo1.md).

## GetAll
- Назначение: вызывает `/usr/lib/alterator/backends/systeminfo --all` и возвращает агрегированный набор значений `HOSTNAME`, `OS_NAME`, `BRANCH`, `KERNEL`, `CPU`, `ARCH`, `GPU`, `MEMORY`, `DRIVE`, `MOTHERBOARD`, `MONITOR`.
- Формат вывода: каждая строка имеет вид `КЛЮЧ="значение"`; ключи перечисляются в указанном порядке.
- Коды возврата: `0` — успех; `!= 0` — ошибка выборки любой составляющей.

## GetCPU
- Назначение: выполняет `systeminfo cpu`, который извлекает модель CPU, количество логических ядер и максимальную частоту из `/proc/cpuinfo` и sysfs.
- Параметры: аргументы отсутствуют; возвращает `stdout_strings` и `response`.
- Формат `stdout_strings`: строго упорядоченные строки `[0]` — модель CPU, `[1]` — количество логических ядер, `[2]` — частота в мегагерцах.
- Коды возврата: `0` — успех; `!= 0` — ошибка выборки.

## GetMotherboard
- Назначение: вызывает `systeminfo motherboard`, объединяя содержимое `/sys/devices/virtual/dmi/id/board_{vendor,name,version}`.
- Параметры: аргументы отсутствуют; возвращает `stdout_strings` и `response`.
- Формат `stdout_strings`: строго упорядоченные строки `[0]` — производитель платы, `[1]` — модель, `[2]` — ревизия.
- Коды возврата: `0` — успех; `!= 0` — ошибка считывания DMI.

## GetLocale
- Назначение: запускает `systeminfo locale`, который считывает ключ `LANG` из `/etc/locale.conf`.
- Параметры: аргументы отсутствуют; возвращает `stdout_strings` и `response`.
- Формат `stdout_strings`: строка локали в виде `<язык>_<регион>.<кодировка>` (например, `ru_RU.UTF-8`).
- Коды возврата: `0` — успех; `!= 0` — ошибка чтения конфигурации.

## ListDesktopEnvironments
- Назначение: выполняет `systeminfo list-desktop-environments`, перебирая `.desktop` файлы окружений рабочего стола.
- Параметры: аргументы отсутствуют; возвращает `stdout_strings` и `response`.
- Формат `stdout_strings`: список установленных окружений, каждое значение в отдельной строке. Возможные элементы — `CINNAMON`, `GNOME`, `KDE<номер_версии>`, `MATE`, `XFCE`.
- Коды возврата: `0` — успех; `!= 0` — ошибка обхода файловой системы.
