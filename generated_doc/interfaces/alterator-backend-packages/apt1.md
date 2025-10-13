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
