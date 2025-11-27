**alterator-backend-systeminfo**

[English](README.md) | [Русский](README.ru_RU.md)

Предназначен для предоставления клиентам Alterator сведений о системе через интерфейс D-Bus.

---

# Общие сведения
| Компонент | Расположение | Назначение |
| --------- | ------------ | ---------- |
| Скрипт `systeminfo` | `/usr/lib/alterator/backends/systeminfo` | Предоставляет системную информацию о компьютере и операционной системе. |
| Файл `systeminfo.backend` | `/usr/share/alterator/backends/systeminfo.backend` | Описывает интерфейс `org.altlinux.alterator.systeminfo1` объекта `/org/altlinux/alterator/systeminfo`. |
| Файл `systeminfo.object` | `/usr/share/alterator/objects/systeminfo.object` | Содержит сведения для регистрации объекта Alterator «О системе». |
| Скрипт `systeminfo.d/notes` | `/usr/lib/alterator/backends/systeminfo.d/notes` | Предназначен для использования скриптом `systeminfo` при поиске лицензий, заметок релиза и записей о дистрибутиве. |

# Возможности
- Получение текстовых характеристик системы: имя хоста, ветка репозитория, версия ядра, локаль.
- Получение аппаратных сведений: CPU, GPU, объём памяти, накопители, мониторы, материнская плата.
- Возврат содержимого лицензии, заметок релиза и финальных инструкций с учётом локали.
- Предоставление сводной информации о машине и системе для метода `GetAll`.
- Определение установленных окружений рабочего стола.

# Интеграция с другими компонентами
- Методы для получения информации о машине и системе доступны клиентам через интерфейс `org.altlinux.alterator.systeminfo1`.
- Регистрацию backend выполняет `alterator-module-executor` в составе `alterator-manager` на основании `/usr/share/alterator/backends/systeminfo.backend`.
- Метод `GetLicense` и связанные команды учитывают наличие редакции: `systeminfo.d/notes` обращается к `/usr/lib/alterator/backends/edition`, при необходимости выполняет поиск файлов по каталогам лицензий и заметок.
- Файл `systeminfo.object` включает объект категории `X-Alterator-System`, что позволяет отображать модуль в `alterator-explorer`.

# Команды `systeminfo`
| Команда | Выходные данные | Источники |
| ------- | ---------------- | --------- |
| `host-name` | Имя узла. | Команда `hostname`. |
| `os-name`, `branch`, `kernel`, `arch` | Текстовые строки с общими сведениями системы. | `/etc/os-release`, rpm-макрос `%_priority_distbranch`, `/proc/sys/kernel/osrelease`, команда `arch`. |
| `os-license`, `release-notes`, `final-notes` | Содержимое HTML-файлов лицензии и заметок. | Поиск через `systeminfo.d/notes`: для систем с редакцией сначала обращается `/usr/lib/alterator/backends/edition` и каталогу `/usr/share/alt-notes`; при отсутствии редакции файлы ищутся последовательно в `/usr/share/alt-notes`, `/usr/share/alt-license`, `/var/lib/install3/licenses` с подстановкой локали или `license.all.html`. |
| `locale`, `list-desktop-environments` | Текущая локаль, перечень окружений рабочего стола. | Чтение `/etc/locale.conf`, обход `.desktop` файлов окружений. |
| `cpu`, `gpu`, `memory`, `drive`, `motherboard`, `monitor` | Аппаратные сведения, обработанные из `/proc` и `/sys`. | Чтение системных файлов, вывод `lspci`/`lscpu`, команда `arch`. |

Важно: все команды выполняются с `set -o pipefail`, ошибки приводят к коду возврата `!= 0` и диагностике в stderr.

# Документация по интерфейсам
- `systeminfo1` — предоставляет сведения об установленной системе, включая характеристики оборудования и служебные заметки. См. [systeminfo1.md](./systeminfo1.md).
