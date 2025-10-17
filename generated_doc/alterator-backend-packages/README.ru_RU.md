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

# Порядок проверки
1. Установить пакет `alterator-backend-packages` и убедиться, что файлы backend и объекты размещены в каталоге `/usr/share/alterator/`.
2. Через `dbus-send` вызвать `org.altlinux.alterator.apt1.Info` и проверить, что описание объекта получено без ошибок.
3. Выполнить `dbus-send` метода `org.altlinux.alterator.apt1.CheckApply` с тестовым списком и убедиться, что возвращается корректный JSON.
4. Вызвать `org.altlinux.alterator.repo1.List` и убедиться, что вывод совпадает с результатом `apt-repo list`.
5. Проверить `org.altlinux.alterator.rpm1.PackageInfo` для известного пакета и сопоставить вывод с `rpm -qi`.

# Документация по интерфейсам
- `apt1` — управляет операциями `apt-get` и `apt-wrapper`, включая обновления индекса, транзакции и проверки сценариев. См. [apt1.md](./apt1.md).
- `repo1` — обращается к `apt-repo` для перечисления, добавления и удаления источников пакетов. См. [repo1.md](./repo1.md).
- `rpm1` — выполняет операции `rpm` и `rpm-wrapper` над локальными пакетами: перечень, установка, удаление, инспекция. См. [rpm1.md](./rpm1.md).
