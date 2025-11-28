**alterator-backend-systeminfo**

[English](README.md) | [Русский](README.ru_RU.md)

Предназначен для предоставления клиентам Alterator сведений о системе через интерфейс D-Bus.

---

# Общие сведения
| Компонент | Расположение | Назначение |
| --------- | ------------ | ---------- |
| Скрипт `systeminfo` | `/usr/lib/alterator/backends/systeminfo` | Предоставляет системную информации о компьютере и операционной системе. |
| Файл `systeminfo.backend` | `/usr/share/alterator/backends/systeminfo.backend` | Описывает интерфейс org.altlinux.alterator.systeminfo1 объекта `/org/altlinux/alterator/systeminfo`. |
| Файл `systeminfo.object` | `/usr/share/alterator/objects/systeminfo.object` | Содержит сведения для регистрации объекта Alterator «О системе» для клиентов. |
| Скрипт `systeminfo.d/notes` | `/usr/lib/alterator/backends/systeminfo.d/notes` | Предоставляет вспомогательные функции поиска лицензий, заметок релиза и записей информации о дистрибутиве для использования скриптом `systeminfo`. |

# Возможности
- Получение текстовых характеристик системы: имя хоста, ветка репозитория, версия ядра, локаль.
- Выдача аппаратных сведений: CPU, GPU, объём памяти, накопители, мониторы, материнская плата, архитектура.
- Возврат содержимого лицензии, заметок релиза и финальных инструкций установки с учётом локали.
- Формирование агрегированного вывода для команды `GetAll` с системными и аппаратными сведениями.
- Вывод списка установленных окружений рабочего стола.

# Интеграция с другими компонентами
- Методы для получения информации о машине и системе доступны клиентам через интерфейс `org.altlinux.alterator.systeminfo1`.
- Регистрацию backend выполняет `alterator-module-executor` в составе `alterator-manager` на основании `/usr/share/alterator/backends/systeminfo.backend`.
- Поиск лицензии, release-notes и final-notes выполняется через `systeminfo.d/notes` с учётом наличия редакции: сначала проверяются файлы, предоставленные `/usr/lib/alterator/backends/edition` (при установленном `alterator-backend-edition-utils`), затем последовательно просматриваются `/usr/share/alt-notes/license.<язык>.html`, `/usr/share/alt-license/license.<язык>.html`, `/var/lib/install3/licenses/license.<язык>.html`, при отсутствии локализованной версии используется `license.all.html` из указанных путей.
- Файл `systeminfo.object` включает объект категории `X-Alterator-System`, что позволяет отображать модуль в `alterator-explorer`.

# Команды `systeminfo`
| Команда | Выходные данные | Источники |
| ------- | ---------------- | --------- |
| `host-name`, `os-name`, `branch`, `kernel` | Текстовые строки с общими сведениями системы. | `hostname`, `/etc/os-release`, rpm-макрос `%_priority_distbranch`, `/proc/sys/kernel/osrelease`. |
| `os-license`, `release-notes`, `final-notes` | Содержимое HTML-файлов лицензии и заметок. | Поиск через `systeminfo.d/notes`: для системы без редакции — последовательный просмотр `/usr/share/alt-notes/license.<язык>.html`, `/usr/share/alt-license/license.<язык>.html`, `/var/lib/install3/licenses/license.<язык>.html`, с подстановкой `license.all.html` при отсутствии локализованного файла; при наличии `alterator-backend-edition-utils` сначала используется `/usr/lib/alterator/backends/edition` и каталоги `/usr/share/alt-notes`. |
| `locale`, `list-desktop-environments` | Текущая локаль, перечень окружений рабочего стола. | Чтение `/etc/locale.conf`, обход `.desktop` файлов окружений. |
| `arch`, `cpu`, `gpu`, `memory`, `drive`, `motherboard`, `monitor` | Аппаратные сведения, обработанные из `/proc` и `/sys`. | Чтение системных файлов и вывод `arch`, `lspci`/`lscpu`. |

Важно: все команды выполняются с `set -o pipefail`, ошибки приводят к коду возврата `!= 0` и диагностике в stderr.

# Документация по интерфейсам
- `systeminfo1` — предоставляет сведения об установленной системе, включая характеристики оборудования и служебные заметки. См. [systeminfo1.md](./systeminfo1.md).
