**alterator-backend-packages**

[Русский](README.ru_RU.md)

Предназначен для предоставления backend-скриптов Alterator, управляющих пакетами, репозиториями и установкой локальных RPM через D-Bus.

---

# Общие сведения
| Компонент | Расположение | Назначение |
| --------- | ------------ | ---------- |
| Файл `apt.backend` | `/usr/share/alterator/backends/apt.backend` | Описывает backend интерфейса `org.altlinux.alterator.apt1` и связывает методы с утилитами `apt-get` и `apt-wrapper`. |
| Скрипт `apt-wrapper` | `/usr/lib/alterator-backend-packages/apt-wrapper` | Выполняет транзакции установки и удаления, проверки сценариев и поиск пакетов с формированием JSON-ответов. |
| Скрипты `alterator-logger.*` | `/usr/share/apt/scripts/alterator-logger.lua`, `/etc/apt/apt.conf.d/alterator-logger.conf`, `/etc/logrotate.d/alterator-logger.logrotate` | Регистрируют обработчик dist-upgrade и ведут журнал `/var/log/alterator/apt/dist-upgrades.log` с ротацией. |
| Файл `rpm.backend` | `/usr/share/alterator/backends/rpm.backend` | Реализует интерфейс `org.altlinux.alterator.rpm1` для операций с локальными RPM-пакетами. |
| Скрипт `rpm-wrapper` | `/usr/lib/alterator-backend-packages/rpm-wrapper` | Предоставляет команду `list` для получения перечня установленных пакетов с метаданными. |
| Файл `repo.backend` | `/usr/share/alterator/backends/repo.backend` | Определяет интерфейс `org.altlinux.alterator.repo1` для управления источниками `apt-repo`. |
| Файлы `*.object` | `/usr/share/alterator/objects/` | Регистрируют объекты Alterator `amp-apt`, `amp-rpm`, `amp-repo` в категории `componentsCategory`. |
| Файлы `*.xml`, `*.policy` | `/usr/share/dbus-1/interfaces/`, `/usr/share/polkit-1/actions/` | Публикуют D-Bus-интерфейсы и политики Polkit для авторизации вызовов методов. |

# Возможности
- Обновление индекса пакетов, запуск `dist-upgrade`, установка, переустановка и удаление пакетов в асинхронном режиме с передачей stdout/stderr через сигналы.
- Проверка транзакций `apt-get` без применения, сбор списков устанавливаемых и удаляемых пакетов, расчёт дополнительных удалений.
- Вывод справочных данных: время последнего `apt-get update`, время последнего `dist-upgrade`, полный перечень доступных пакетов и поиск по шаблону.
- Управление репозиториями `apt-repo`: перечисление источников, добавление и удаление записей.
- Работа с локальными RPM-файлами: установка по пути, удаление по имени пакета, получение информации и состава файлов.
- Ведение журнала успешных `dist-upgrade` и подготовка конфигурации для logrotate.

# Интеграция с Alterator
- Все backend-файлы используют модуль `executor`, что позволяет `alterator-manager` запускать системные команды от имени службы Alterator.
- Объекты `amp-apt`, `amp-rpm`, `amp-repo` публикуются в категории `componentsCategory`, что делает модули доступными клиентам Alterator.
- Интерфейсы `apt1`, `rpm1`, `repo1` устанавливаются в `/usr/share/dbus-1/interfaces/`, а действия Polkit требуют аутентификацию при вызове операций изменения.
- Асинхронные методы `UpdateAsync`, `ApplyAsync`, `ReinstallAsync`, `DistUpgradeAsync` назначают сигналы stdout/stderr для потоковой передачи вывода клиентам.
- APT-обработчик `alterator-logger.lua` подключается через `apt.conf.d` и пишет метки времени в журнал `dist-upgrades.log` для последующего чтения методами backend.

# Методы интерфейса `apt1`
| Метод | Выполняемая команда | Назначение |
| ----- | ------------------- | ---------- |
| `Info` | `cat /usr/share/alterator/objects/apt.object` | Возвращает описание объекта Alterator с локализацией. |
| `UpdateAsync` | `apt-get update -q` | Обновляет индексы пакетов и транслирует ход выполнения по сигнальным каналам. |
| `ApplyAsync` | `apt-wrapper apply {exclude_pkgnames} {pkgnames}` | Запускает транзакцию установки/удаления с учётом списка исключений и временного `pkgpriorities`. |
| `ReinstallAsync` | `apt-get reinstall -y -q {pkgnames}` | Переустанавливает выбранные пакеты в фоновом режиме. |
| `DistUpgradeAsync` | `apt-get dist-upgrade -y -q` | Выполняет обновление дистрибутива с выводом прогресса. |
| `ListAllPackages` | `apt-wrapper listall` | Выдаёт полный список доступных пакетов (до 10 МБ данных). |
| `Search` | `apt-wrapper search {pattern}` | Выполняет поиск пакетов по регулярному выражению. |
| `LastUpdate` | `apt-wrapper lastupdate` | Сообщает время последнего обновления индекса (`stat` каталога списков). |
| `LastDistUpgrade` | `apt-wrapper lastdistupgrade` | Сообщает время последнего `dist-upgrade` из журнала. |
| `CheckApply` | `apt-wrapper check-apply {pkgnames}` | Проводит симуляцию транзакции и возвращает JSON со списками установки и удаления. |
| `CheckReinstall` | `apt-wrapper check-reinstall {pkgnames}` | Проверяет возможность переустановки и возвращает диагностический вывод. |
| `CheckDistUpgrade` | `apt-wrapper check-dist-upgrade` | Выполняет проверку сценария `dist-upgrade` без изменения системы. |

Примечание: вспомогательные команды `check-install` и `check-remove` доступны напрямую в `apt-wrapper` и используются клиентами для расширенной диагностики.

# Методы интерфейса `repo1`
| Метод | Выполняемая команда | Назначение |
| ----- | ------------------- | ---------- |
| `Info` | `cat /usr/share/alterator/objects/repo.object` | Возвращает описание объекта «Источники пакетов». |
| `List` | `apt-repo list` | Перечисляет доступные репозитории apt. |
| `Add` | `apt-repo add {source}` | Добавляет новый источник репозитория. |
| `Remove` | `apt-repo rm {source}` | Удаляет источник из конфигурации. |

# Методы интерфейса `rpm1`
| Метод | Выполняемая команда | Назначение |
| ----- | ------------------- | ---------- |
| `Info` | `cat /usr/share/alterator/objects/rpm.object` | Возвращает описание объекта RPM. |
| `List` | `rpm-wrapper list` | Даёт список установленных пакетов с версией, релизом, архитектурой и группой. |
| `Install` | `rpm -U {pkgpath}` | Устанавливает или обновляет пакет из указанного файла. |
| `Remove` | `rpm -e {pkgname}` | Удаляет установленный пакет по имени. |
| `PackageInfo` | `rpm -qi {pkgname}` | Выводит сведения об установленном пакете. |
| `Files` | `rpm -ql {pkgname}` | Перечисляет файлы, входящие в пакет. |

# Порядок проверки
1. Установить пакет `alterator-backend-packages` и убедиться, что файлы backend и объекты размещены в каталоге `/usr/share/alterator/`.
2. Через `dbus-send` вызвать `org.altlinux.alterator.apt1.Info` и проверить, что описание объекта получено без ошибок.
3. Выполнить `dbus-send` метода `org.altlinux.alterator.apt1.CheckApply` с тестовым списком и убедиться, что возвращается корректный JSON.
4. Вызвать `org.altlinux.alterator.repo1.List` и убедиться, что вывод совпадает с результатом `apt-repo list`.
5. Проверить `org.altlinux.alterator.rpm1.PackageInfo` для известного пакета и сопоставить вывод с `rpm -qi`.

# Документация по интерфейсам
- [Интерфейс `apt1`](./apt1.md)
- [Интерфейс `repo1`](./repo1.md)
- [Интерфейс `rpm1`](./rpm1.md)
