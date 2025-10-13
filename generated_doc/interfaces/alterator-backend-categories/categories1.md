## Общие сведения

Интерфейс org.altlinux.alterator.categories1 предоставляет операции чтения категорий Alterator. Методы обращаются к скриптам `/usr/lib/alterator-backend-categories/category-info` и `.../list-categories`, которые поддерживают форматы TOML и legacy Desktop Entry.

## Info

- Назначение: выполняет `category-info {name}` для чтения описания категории.
- Параметры: принимает строку name; возвращает stdout_bytes и response.
- Ожидаемое поведение (пример): при наличии TOML-файла в `/usr/share/alterator/categories` stdout_bytes содержит его содержимое; при отсутствии используется файл `.desktop` из `/usr/share/alterator/desktop-directories`, который конвертируется в TOML с полями `name`, `display_name`, `comment`, `icon`, `weight`. Ненайденная категория приводит к ненулевому response.

## List

- Назначение: вызывает `list-categories` для перечисления идентификаторов категорий.
- Параметры: аргументов нет; возвращает stdout_strings и response.
- Ожидаемое поведение (пример): stdout_strings содержит список имён категорий из новых TOML-файлов и legacy `.desktop`; response = 0.
