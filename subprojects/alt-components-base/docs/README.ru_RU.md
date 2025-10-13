**alt-components-base**

[Русский](README.ru_RU.md) | [English](../README.md)

Предназначен для формирования иерархической структуры системных компонентов.

---

# Общие сведения
В рамках проекта используется 3 сущности alterator-entry:
| Сущность  |    Формат   | Описание   |
| ----------| ------------| -----------|
| Компонент | `.component`| Набор логически объединенных rpm-пакетов|
| Категория | `.category` | Сущность для группировки компонентов по тематическому признаку|
| Редакция  | `.edition`  | Сущность высшего уровня, предназначенная для формирования целевых конфигураций дистрибутива|

Сущности описываются в формате TOML и должны строго соответсвовать спецификации, описанной в проекте [alterator-entry](https://altlinux.space/alterator/alterator-entry/src/branch/master/doc). Также там приводятся и [примеры написания компонентов](https://altlinux.space/alterator/alterator-entry/src/branch/master/examples/component).

Ссылки на спецификации сущностей:
- [Component](https://altlinux.space/alterator/alterator-entry/src/branch/master/doc#%D1%81%D1%83%D1%89%D0%BD%D0%BE%D1%81%D1%82%D1%8C-%D1%82%D0%B8%D0%BF%D0%B0-component)
- [Category](https://altlinux.space/alterator/alterator-entry/src/branch/master/doc#%D1%81%D1%83%D1%89%D0%BD%D0%BE%D1%81%D1%82%D1%8C-%D1%82%D0%B8%D0%BF%D0%B0-category)
- [Edition](https://altlinux.space/alterator/alterator-entry/src/branch/master/doc#%D1%81%D1%83%D1%89%D0%BD%D0%BE%D1%81%D1%82%D1%8C-%D1%82%D0%B8%D0%BF%D0%B0-edition)

# Взаимосвязь с клиентами
Связь между компонентами (категориями и разделами) и клиентами (например, графическим приложением [alt-components](https://altlinux.space/alterator/alt-components) или модулем components утилиты [alteratorctl](https://altlinux.space/alterator/alteratorctl)) организовано через D-Bus - систему межпроцессорного взаимодействия.

Однако проект alt-components-base напрямую не предоставляет интерфейс для взаимедойствия, этим занимается пакет [alterator-backend-component](https://altlinux.space/alterator/alterator-backend-component):

1. Пакет `alterator-backend-component` содержит filetrigger, который при добавлении в систему новых компонентов (находятся в `/usr/share/alterator/components`) вызывает скрипт `/usr/lib/alterator/backends/component.d/generate-components-backends`, генерирующий backend-файлы, необходимые для [alterator-manager](https://altlinux.space/alterator/alterator-manager).

2. `alterator-manager` считывает новые backend-файлы и генерирует объекты `/org/altlinux/alterator/<component_name>` на D-Bus, реализующие интерфейс `org.altlinux.alterator.component1`.

Таким образом, разработчику компонентов не нужно беспокоиться о создании объекта и реализации интерфейса - достаточно добавить необходимые файлы (`.component`, `description.md` и т.д.) в alt-components-base.

> Для оптимизации работы с компонентами и категориями на D-Bus представлен объект `/org/altlinux/alterator/global`, предоставляющий интерфейсы взаимодействия с ними (`org.altlinux.alterator.batch_component_categories1`, `org.altlinux.alterator.batch_components1`, `org.altlinux.alterator.component_categories1`, `org.altlinux.alterator.current_edition1`)

> Перечисленные интерфейсы предоставляются пакетом `alterator-backend-component`

# Как добавить свой компонент в проект
1. Ответвиться от проекта.
2. Добавить свои файлы сущностей (.component, .category, .edition) в проект.
3. Проверить валидноcть (скрипт `./scripts/validate_categories.py`).
4. Если валидация прошла успешно - прислать PR в репозиторий

> Перед отправкой PR рекомендуется проверить изменения в тестовой среде (с использованием клиентов `alt-components` и `alteratorctl components`)
