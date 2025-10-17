**alterator-backend-categories**

[Русский](README.ru_RU.md)

Предназначен для предоставления клиентам Alterator списка категорий и их описаний.

---

# Общие сведения
| Компонент | Расположение | Назначение |
| --------- | ------------ | ---------- |
| Скрипт `category-info` | `/usr/lib/alterator-backend-categories/category-info` | Возвращает описание категории в формате TOML. |
| Скрипт `list-categories` | `/usr/lib/alterator-backend-categories/list-categories` | Выводит доступные категории Alterator. |
| Файл `categories.backend` | `/etc/alterator/backends/categories.backend` | Реализует backend интерфейса `org.altlinux.alterator.categories1`. |
| Файлы категорий | `/usr/share/alterator/categories`, `/usr/share/alterator/desktop-directories` | Источники данных нового и наследуемого формата. |

# Возможности
- Чтение категорий нового формата (`*.category`) с использованием библиотеки `alterator_entry`.
- Поддержка устаревших INI-файлов (`*.desktop`) из каталога `desktop-directories` с конвертацией в TOML.
- Выдача локализованных заголовков и комментариев через обработку ключей `Name[]` и `Comment[]`.
- Расчёт веса категории на основе встроенной таблицы при преобразовании из старого формата.
- Формирование списка категорий с объединением данных из обоих источников.

# Работа скрипта `category-info`
- При запросе нового формата выводит содержимое файла `.category` без изменений.
- Для наследуемых INI-файлов преобразует секцию `Desktop Entry` в структуру TOML со свойствами `type`, `name`, `display_name`, `comment`, `icon`, `weight`.
- Учитывает локаль (`LC_ALL`) при формировании ключей `display_name` и `comment`, нормализуя региональные коды (`ru_RU`).

# Порядок проверки
1. Разместить тестовую категорию в `/usr/share/alterator/categories/<name>.category` или `desktop-directories`.
2. Выполнить `list-categories` и убедиться, что имя категории присутствует в выводе.
3. Вызвать `category-info <name>` и проверить корректность структуры TOML.
4. Запросить метод `List` интерфейса через `dbus-send` и убедиться в получении объединённого списка.

# Документация по интерфейсам
- `categories1` — выдаёт список категорий Alterator и описания отдельных элементов через скрипты `category-info` и `list-categories`. См. [categories1.md](./categories1.md).
