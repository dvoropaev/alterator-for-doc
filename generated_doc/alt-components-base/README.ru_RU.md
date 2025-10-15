**alterator-backend-packages**

[Русский](README.ru_RU.md) | [English](../README.md)

alterator-backend-packages — набор бекендов alterator, предоставляющих D-Bus-интерфейсы для управления пакетами, APT-репозиториями и локальными RPM.

---

# Общие сведения
alterator-backend-packages предназначен для выполнения привилегированных операций установки, обновления и удаления пакетов по запросу модулей alterator. Бекенды описаны в TOML-файлах `.backend` (модуль `executor`) и используют вспомогательные скрипты из каталога `/usr/lib/alterator-backend-packages`.

Состав поставки:
- `apt/` — инструменты работы с APT: генерация pkgpriorities, вызовы apt-get через `apt-wrapper`, публикация политики polkit и XML-описания интерфейса `org.altlinux.alterator.apt1`.
- `repo/` — управление источниками apt через `apt-repo`, определяет интерфейс `org.altlinux.alterator.repo1`.
- `rpm/` — операции с локальными пакетами rpm посредством `rpm-wrapper` и системной утилиты `rpm`, экспортирует интерфейс `org.altlinux.alterator.rpm1`.
- `po/` — файлы переводов сообщений для текстов, возвращаемых скриптами и бекендами.

Важно: каталоги `apt/logger` содержат lua-скрипты и конфигурацию logrotate для записи журналов `apt-wrapper` в `/var/log/alterator/apt`.

# Взаимодействие с клиентами
alterator-manager загружает `.backend`-описания из `/usr/share/alterator/backends` и формирует объекты `/org/altlinux/alterator/{apt,rpm,repo}`. Клиенты (графические модули alterator, `alteratorctl`) обращаются к интерфейсам `org.altlinux.alterator.apt1`, `org.altlinux.alterator.repo1`, `org.altlinux.alterator.rpm1` по D-Bus. Асинхронные методы используют сигналы для передачи stdout/stderr, синхронные возвращают массивы строк или байтовый поток.

Обработка запросов реализуется обёртками (`apt-wrapper`, `rpm-wrapper`) и системными инструментами `apt-get`, `apt-repo`, `rpm`. Бекенды задают идентификаторы действий (`action_id`) для журналирования и сопоставления с политиками polkit, что позволяет клиентам получать права через polkit-агента.

# Интерфейсы D-Bus
## org.altlinux.alterator.apt1
## Общие сведения

Интерфейс org.altlinux.alterator.apt1 предоставляет методы управления пакетами через apt-get и вспомогательный скрипт apt-wrapper, а также сигналы для потоков stdout/stderr при длительных операциях. Во всех методах поле response содержит код возврата запущенной команды: 0 — успех, ненулевое значение — ошибка.

## Info

- Назначение: читает описание объекта APT из файла /usr/share/alterator/objects/apt.object для публикации метаданных интерфейса.
- Параметры: входных аргументов нет; возвращает stdout_bytes и response.
- Ожидаемое поведение (пример): успешный вызов возвращает содержимое файла apt.object в stdout_bytes и завершает команду с response = 0.

## UpdateAsync

- Назначение: выполняет `apt-get update -q` для синхронизации индексов пакетов.
- Параметры: входных аргументов нет; возвращает response, прогресс поступает через сигналы `apt1_update_stdout_signal` и `apt1_update_stderr_signal`.
- Ожидаемое поведение (пример): при обновлении репозиториев stdout/stderr транслируются в сигналы, по завершении без ошибок response равен 0; метод прерывается по таймауту 86400 секунд.

## ApplyAsync

- Назначение: запускает `/usr/lib/alterator-backend-packages/apt-wrapper apply {exclude_pkgnames} {pkgnames}` для установки и удаления пакетов с учётом списка исключений.
- Параметры: принимает строки exclude_pkgnames и pkgnames; возвращает response, а текстовые потоки передаются сигналами `apt1_install_stdout_signal` и `apt1_install_stderr_signal`.
- Ожидаемое поведение (пример): helper формирует временный pkgpriorities из вручную установленных пакетов (без переданных к удалению) и запускает `apt-get install -y -q`; при успехе response = 0, вывод содержит сообщение «Preparing the transaction…» и ход установки.

## ReinstallAsync

- Назначение: вызывает `apt-get reinstall -y -q` для переустановки указанного списка пакетов.
- Параметры: принимает строку pkgnames; возвращает response, а stdout/stderr транслируются сигналами `apt1_reinstall_stdout_signal` и `apt1_reinstall_stderr_signal`.
- Ожидаемое поведение (пример): корректная переустановка завершает apt-get с кодом 0 и отправляет ход операции через сигналы.

## ListAllPackages

- Назначение: выполняет `/usr/lib/alterator-backend-packages/apt-wrapper listall`, который использует `apt-cache search . --names-only` для получения полного перечня имён пакетов.
- Параметры: не принимает аргументов; возвращает stdout_strings, stderr_strings и response. Ограничение массива stdout_strings — 10 МБ.
- Ожидаемое поведение (пример): stdout_strings содержит список пакетов построчно, stderr_strings пуст, response = 0.

## Search

- Назначение: запускает `/usr/lib/alterator-backend-packages/apt-wrapper search {pattern}` для поиска пакетов по шаблону через apt-cache.
- Параметры: принимает строку pattern; возвращает stdout_strings, stderr_strings и response.
- Ожидаемое поведение (пример): найденные пакеты выводятся в stdout_strings, сообщения об ошибках — в stderr_strings; response равен 0 при успешном завершении apt-cache.

## LastUpdate

- Назначение: вызывает helper `apt-wrapper lastupdate` для получения отметки времени каталога `/var/lib/apt/lists` в формате `YYYY-MM-DD HH:MM:SS UTC`.
- Параметры: входных аргументов нет; возвращает stdout_strings, stderr_strings и response.
- Ожидаемое поведение (пример): при наличии каталога stdout_strings содержит одну строку с датой в UTC, stderr_strings пуст, response = 0; отсутствие индексов приводит к ненулевому коду.

## LastDistUpgrade

- Назначение: запускает helper `apt-wrapper lastdistupgrade`, который читает последнюю строку журнала `/var/log/alterator/apt/dist-upgrades.log`.
- Параметры: аргументы не требуются; возвращает stdout_strings, stderr_strings и response.
- Ожидаемое поведение (пример): при наличии журнала stdout_strings включает последнюю запись dist-upgrade, response = 0; если файл отсутствует, helper завершится с ошибкой и заполнит stderr_strings.

## CheckApply

- Назначение: выполняет `/usr/lib/alterator-backend-packages/apt-wrapper check-apply {pkgnames}` для симуляции транзакции `apt-get install --just-print` с учётом pkgpriorities.
- Параметры: принимает строку pkgnames; возвращает stdout_strings, stderr_strings и response (таймаут 86400 секунд).
- Ожидаемое поведение (пример): stdout_strings содержит JSON с полями `install_packages`, `remove_packages`, `extra_remove_packages`; stderr_strings заполняется при ошибках симуляции, response = 0 при корректной проверке.

## CheckReinstall

- Назначение: вызывает helper `apt-wrapper check-reinstall {pkgnames}`, который запускает `apt-get reinstall -s -q` для оценки переустановки.
- Параметры: принимает строку pkgnames; возвращает stdout_strings, stderr_strings и response (таймаут 86400 секунд).
- Ожидаемое поведение (пример): stdout_strings содержит имена пакетов, которые будут переустановлены, stderr_strings — перечень пакетов на удаление; response = 0 при успешной симуляции.

## CheckDistUpgrade

- Назначение: выполняет `/usr/lib/alterator-backend-packages/apt-wrapper check-dist-upgrade`, который запускает `apt-get dist-upgrade -s -q`.
- Параметры: аргументы отсутствуют; возвращает stdout_strings, stderr_strings и response (таймаут 86400 секунд).
- Ожидаемое поведение (пример): stdout_strings включает пакеты на установку, stderr_strings — пакеты на удаление; response = 0 при успешной оценке.

## DistUpgradeAsync

- Назначение: запускает `apt-get dist-upgrade -y -q` для обновления системы.
- Параметры: входных аргументов нет; возвращает response, а ход операции передаётся сигналами `apt1_dist_upgrade_stdout_signal` и `apt1_dist_upgrade_stderr_signal`.
- Ожидаемое поведение (пример): при успешном завершении dist-upgrade response = 0; ошибки транслируются в сигналы и ненулевой код.

## org.altlinux.alterator.repo1
## Общие сведения

Интерфейс org.altlinux.alterator.repo1 обеспечивает вызовы утилиты apt-repo для управления списками источников APT. Все методы возвращают код response процесса apt-repo: 0 — успешно, ненулевой — ошибка.

## Info

- Назначение: читает описание объекта репозиториев из файла /usr/share/alterator/objects/repo.object.
- Параметры: входных аргументов нет; возвращает stdout_bytes и response.
- Ожидаемое поведение (пример): stdout_bytes содержит TOML-описание репозитория, response = 0.

## List

- Назначение: запускает `apt-repo list` для перечисления подключённых источников.
- Параметры: аргументы отсутствуют; возвращает stdout_strings, stderr_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит построчный список URL/alias из apt-repo, stderr_strings пуст при успехе, response = 0.

## Add

- Назначение: вызывает `apt-repo add {source}` для добавления источника репозиториев.
- Параметры: принимает строку source; возвращает stderr_strings и response.
- Ожидаемое поведение (пример): при корректном добавлении stderr_strings пуст, response = 0; ошибки apt-repo отражаются в stderr_strings и ненулевом коде.

## Remove

- Назначение: выполняет `apt-repo rm {source}` для удаления источника.
- Параметры: принимает строку source; возвращает stderr_strings и response.
- Ожидаемое поведение (пример): успешное удаление приводит к response = 0, диагностические сообщения выводятся в stderr_strings.

## org.altlinux.alterator.rpm1
## Общие сведения

Интерфейс org.altlinux.alterator.rpm1 предназначен для управления локальными пакетами через утилиту rpm и вспомогательный скрипт `rpm-wrapper`. Методы возвращают код завершения запущенной команды в поле response: 0 — операция выполнена, ненулевое значение сигнализирует об ошибке.

## Info

- Назначение: читает описание объекта RPM из файла `/usr/share/alterator/objects/rpm.object`.
- Параметры: аргументы отсутствуют; возвращает `stdout_bytes` и `response`.
- Ожидаемое поведение (пример): при успешном чтении stdout_bytes содержит содержимое `rpm.object`, response равен 0.

## List

- Назначение: запускает `/usr/lib/alterator-backend-packages/rpm-wrapper list`, который выводит установленный список пакетов в формате `NAME VERSION RELEASE ARCH GROUP`.
- Параметры: аргументы отсутствуют; возвращает `stdout_strings`, `stderr_strings` и `response`.
- Ожидаемое поведение (пример): при корректном выполнении helper печатает перечень пакетов построчно, stderr пуст, response = 0.

## Install

- Назначение: вызывает `rpm -U {pkgpath}` для установки или обновления пакета из указанного файла.
- Параметры: принимает строку `pkgpath`; возвращает `stderr_strings` и `response`.
- Ожидаемое поведение (пример): при корректной установке rpm завершается с кодом 0, диагностические сообщения выводятся в stderr и возвращаются вызывающей стороне.

## Remove

- Назначение: запускает `rpm -e {pkgname}` для удаления установленного пакета.
- Параметры: принимает строку `pkgname`; возвращает `stderr_strings` и `response`.
- Ожидаемое поведение (пример): при успешном удалении rpm завершает работу с response = 0; сообщения о зависимостях передаются через stderr.

## PackageInfo

- Назначение: выполняет `rpm -qi {pkgname}` для получения сведений о пакете.
- Параметры: принимает строку `pkgname`; возвращает `stdout_strings`, `stderr_strings` и `response`.
- Ожидаемое поведение (пример): при наличии пакета stdout_strings содержит текст из `rpm -qi`, stderr пуст, response = 0; отсутствие пакета приводит к ненулевому коду и сообщению об ошибке.

## Files

- Назначение: вызывает `rpm -ql {pkgname}` для перечисления файлов установленного пакета.
- Параметры: принимает строку `pkgname`; возвращает `stdout_strings`, `stderr_strings` и `response`.
- Ожидаемое поведение (пример): при успешном выполнении stdout_strings содержит список путей, stderr пуст, response = 0; ошибки (например, пакет не установлен) приводят к ненулевому коду и описанию в stderr.
