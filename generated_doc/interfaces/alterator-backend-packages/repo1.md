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
