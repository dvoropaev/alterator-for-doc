**alterator-backend-systeminfo**

[Русский](README.ru_RU.md)

Предназначен для предоставления клиентам Alterator сведений о системе через интерфейс D-Bus.

---

# Общие сведения
| Компонент | Расположение | Назначение |
| --------- | ------------ | ---------- |
| Скрипт `systeminfo` | `/usr/lib/alterator/backends/systeminfo` | Выполняет команды выборки сведений об установленной системе. |
| Файл `systeminfo.backend` | `/etc/alterator/backends/systeminfo.backend` | Описывает backend с методами интерфейса `org.altlinux.alterator.systeminfo1`. |
| Файл `systeminfo.object` | `/usr/share/alterator/objects/systeminfo.object` | Регистрирует объект Alterator «О системе» для клиентов. |
| Каталог `systeminfo.d/notes` | `/usr/lib/alterator/backends/systeminfo.d/notes` | Предоставляет вспомогательные функции поиска лицензий и заметок релиза. |

# Возможности
- Получение текстовых характеристик системы: имя хоста, ветка репозитория, версия ядра, локаль.
- Выдача аппаратных сведений: CPU, GPU, объём памяти, накопители, мониторы, материнская плата.
- Предоставление ссылок на лицензию, заметки релиза и финальные инструкции установки с учётом локали.
- Формирование агрегированного вывода по команде `--all` для передачи в `GetAll`.
- Список установленных окружений рабочего стола по команде `list-desktop-environments`.

# Интеграция с другими компонентами
- Backend использует интерфейс `org.altlinux.alterator.systeminfo1` и отдаёт данные клиентам через D-Bus.
- Метод `GetLicense` и связанные команды делегируют поиск файлов модулю `systeminfo.d/notes`, который при наличии вызывает `/usr/lib/alterator/backends/edition` для получения данных текущей редакции.
- Файл `systeminfo.object` включает объект категории `X-Alterator-System`, что позволяет отображать модуль в `alterator-manager`.

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
