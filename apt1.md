# ==========================Версия 1==========================

## Общие сведения

Интерфейс org.altlinux.alterator.apt1 определяет D-Bus методы управления пакетами APT и набор связанных сигналов для потоков stdout/stderr. Во всех методах поле response — целочисленный код возврата запущенной команды: 0 обозначает успешное завершение, любое ненулевое значение сигнализирует об ошибке.

## Info

- Назначение: читает объектное описание APT из файла /usr/share/alterator/objects/apt.object через cat для публикации метаданных и интерфейса.
- Параметры: не принимает входных аргументов; возвращает stdout_bytes (массив байт с содержимым файла) и response.
- Ожидаемое поведение (пример): успешный вызов отдаёт описание объекта (type = "Object", name = "amp-apt", комментарии и т. д.) в stdout_bytes, а response равен 0.

## UpdateAsync

- Назначение: запускает apt-get update -q для обновления индексов пакетов.
- Параметры: входных аргументов нет; возвращает только response.
- Ожидаемое поведение (пример): прогресс и ошибки команды передаются через сигналы apt1_update_stdout_signal/apt1_update_stderr_signal, а после успешного завершения response будет 0.

## ApplyAsync

- Назначение: вызывает helper /usr/lib/alterator-backend-packages/apt-wrapper apply … для установки и удаления пакетов с учётом списка исключений и приоритетов.
- Параметры: принимает строки exclude_pkgnames и pkgnames; возвращает response. Пакеты на удаление помечаются суффиксом - в pkgnames. Строка exclude_pkgnames содержит пробельно разделённый перечень пакетов, которые не требуется учитывать в pkgpriorities; helper дополняет его именами из pkgnames с суффиксом - перед созданием файла приоритетов.
- Ожидаемое поведение (пример): helper формирует список исключений и временный файл pkgpriorities, выводит сообщение «Preparing the transaction…», затем запускает apt-get install -y -q для переданных пакетов; stdout/stderr передаются сигналами apt1_install_stdout_signal/apt1_install_stderr_signal, а по успешному завершению response равен 0.

## ReinstallAsync

- Назначение: выполняет apt-get reinstall -y -q для указанных пакетов.
- Параметры: принимает строку pkgnames; возвращает response.
- Ожидаемое поведение (пример): вывод команды транслируется сигналами apt1_reinstall_stdout_signal/apt1_reinstall_stderr_signal, а response = 0 подтверждает успешную переустановку списка pkgnames.

## ListAllPackages

- Назначение: перечисляет доступные пакеты через helper apt-wrapper listall, который запускает apt-cache search . --names-only.
- Параметры: входных нет; возвращает stdout_strings, stderr_strings и response.
- Ожидаемое поведение (пример): успешный вызов даёт массив stdout_strings со списком имён пакетов (по одной строке на пакет), stderr_strings пуст, response = 0.

## Search

- Назначение: выполняет поиск пакетов по шаблону через apt-wrapper search {pattern}, который вызывает apt-cache search с указанным паттерном.
- Пааметры: принимает строку pattern; возвращает stdout_strings, stderr_strings, response.
- Ожидаемое поведение (пример): список найденных пакетов выводится строками в stdout_strings, сообщения об ошибках — в stderr_strings; response = 0, если apt-cache search завершился без ошибок.

## LastUpdate

- Назначение: сообщает время последнего обновления индексов, вызывая helper apt-wrapper lastupdate, который читает метку времени каталога /var/lib/apt/lists и переводит её в формат YYYY-MM-DD HH:MM:SS UTC.
- Параметры: входных нет; возвращает stdout_strings, stderr_strings, response.
- Ожидаемое поведение (пример): при наличии индексов stdout_strings содержит единственную строку с датой в UTC, stderr_strings пуст, response = 0. Если каталога нет, helper завершится с ошибкой и выведет сообщение в stderr_strings.

## LastDistUpgrade

- Назначение: выдаёт последнюю запись журнала dist-upgrade через helper apt-wrapper lastdistupgrade, читающий последнюю строку /var/log/alterator/apt/dist-upgrades.log.
- Параметры: входных нет; возвращает stdout_strings, stderr_strings, response.
- Ожидаемое поведение (пример): если файл журнала существует, stdout_strings содержит строку с датой и описанием последнего dist-upgrade, response = 0; отсутствие файла приводит к сообщению об ошибке в stderr_strings и ненулевому response.

## CheckApply

- Назначение: эмулирует установку/удаление пакетов с помощью helper apt-wrapper check-apply, который запускает apt-get install --just-print с созданным pkgpriorities.
- Параметры: принимает строку pkgnames; возвращает stdout_strings, stderr_strings, response.
- Ожидаемое поведение (пример): результатом является JSON вида {"install_packages":[...],"remove_packages":[...],"extra_remove_packages":[...]} в stdout_strings, сформированный по выводу apt-get; ошибки симуляции попадают в stderr_strings, response = 0 при чистом прогоне. Поле extra_remove_packages заполняется списком пакетов, удаление которых apt-get предупреждает выполнить сверх явного списка pkgnames (фрагмент между предупреждением «This should NOT be done…» и итоговой строкой о количестве пакетов).

Принцип заполнения extra_remove_packages:

- helper обрезает текст предупреждения и завершающего резюме, оставляя блок с перечислением пакетов для дополнительного удаления;
- каждая строка блока очищается от скобок и лишних пробелов, после чего превращается в отдельный элемент JSON;
- итоговый массив фиксирует имена пакетов, для которых apt-get требует принудительное удаление как зависимостей, не указанных явно.

Источник: [alterator-backend-packages/apt/apt-wrapper](https://github.com/alterator/alterator-backend-packages/blob/master/apt/apt-wrapper).

## CheckReinstall

- Назначение: прогоняет apt-get reinstall -s -q для оценки переустановки пакетов через helper apt-wrapper check-reinstall.
- Параметры: принимает строку pkgnames; возвращает stdout_strings, stderr_strings, response.
- Ожидаемое поведение (пример): helper печатает имена пакетов для переустановки в stdout (становятся stdout_strings), пакеты на удаление — в stderr (stderr_strings); при корректной симуляции response = 0.

## CheckDistUpgrade

- Назначение: анализирует последствия dist-upgrade через helper apt-wrapper check-dist-upgrade, использующий apt-get dist-upgrade -s -q.
- Параметры: не принимает входных аргументов; возвращает stdout_strings, stderr_strings, response.
- Ожидаемое поведение (пример): список пакетов на установку выводится в stdout (stdout_strings), на удаление — в stderr (stderr_strings); response = 0, если apt-get смог построить план обновления без ошибок.

## DistUpgradeAsync

- Назначение: запускает фактическое apt-get dist-upgrade -y -q для обновления системы.
- Параметры: входных аргументов нет; возвращает response.
- Ожидаемое поведение (пример): поток выполнения транслируется сигналами apt1_dist_upgrade_stdout_signal/apt1_dist_upgrade_stderr_signal, а response = 0 означает успешное завершение обновления пакетов.
