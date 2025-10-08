# Code/Commit/Merge Conventions

## Code Conventions

### Общие принципы
- **DRY (Don't Repeat Yourself)** - избегайте дублирования кода
- **KISS (Keep It Simple, Stupid)** - простота превыше сложности
- **YAGNI (You Aren't Gonna Need It)** - не добавляйте функциональность "на будущее"

### Стиль кодирования
```python
# Хорошо
class UserAccountManager:
    def create_user_account(self, username: str, email: str) -> User:
        """Создает новый аккаунт пользователя."""
        if not self._validate_username(username):
            raise InvalidUsernameError(f"Invalid username: {username}")
        
        user = User(username=username, email=email)
        return self._save_user(user)

# Плохо
class UAM:
    def cua(self, u, e):
        if not self.vu(u):
            raise Error()
        usr = User(u, e)
        return self.su(usr)
```

### Именование
- **Классы**: `PascalCase` - `UserService`, `DatabaseConnection`
- **Методы/функции**: `snake_case` - `get_user_by_id()`, `validate_input()`
- **Переменные**: `snake_case` - `user_count`, `max_retries`
- **Константы**: `UPPER_SNAKE_CASE` - `MAX_RETRY_COUNT`, `DEFAULT_TIMEOUT`

### Структура кода
- Максимальная длина строки: **120 символов**
- Отступы: **4 пробела** (не табы)
- Пустые строки между методами/функциями
- Импорты группируются и сортируются

### Обработка ошибок
```python
# Хорошо
try:
    result = database.execute_query(query)
except DatabaseConnectionError as e:
    logger.error(f"Database connection failed: {e}")
    raise ServiceUnavailableError("Service temporarily unavailable")
except InvalidQueryError as e:
    logger.warning(f"Invalid query: {e}")
    raise BadRequestError("Invalid request parameters")

# Плохо
try:
    result = db.query(q)
except:
    pass
```

## Commit Conventions

### Conventional Commits
```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

### Типы коммитов
- **feat**: Новая функциональность
- **fix**: Исправление ошибки
- **docs**: Изменения в документации
- **style**: Изменения форматирования (не влияющие на логику)
- **refactor**: Рефакторинг кода без изменения функциональности
- **test**: Добавление или изменение тестов
- **chore**: Изменения в сборке, зависимостях, конфигурации

### Примеры коммитов
```bash
# Хорошо
feat(auth): добавить двухфакторную аутентификацию

- Добавлена поддержка TOTP
- Интегрирована с существующей системой аутентификации
- Добавлены соответствующие тесты

Closes #123

fix: исправить утечку памяти в кэше

Добавлено освобождение ресурсов при уничтожении объекта кэша.

refactor(user): переработать валидацию email

Использована стандартная библиотека для валидации вместо кастомного решения.

# Плохо
update code
fixed bug
changes
```

### Правила коммитов
- **Атомарность**: один коммит = одно логическое изменение
- **Описательность**: сообщение должно ясно описывать что и зачем изменено
- **Тело коммита**: для сложных изменений добавлять развернутое описание
- **Ссылки на issues**: использовать `Closes #123`, `Fixes #456`

## Merge Conventions

### Структура Pull Request
```markdown
## Описание
Что решает этот PR и почему это важно

## Техническое решение
Краткое описание выбранного подхода и использованных сущностей

## Изменения
- Изменен класс X для поддержки Y
- Добавлен новый метод Z
- Обновлена документация

## Тестирование
- [x] Протестировано локально
- [x] Пройдены существующие тесты
- [x] Добавлены новые тесты

## Связанные Issues
Closes #123
```

### Процесс слияния
1. **Проверка CI**: все проверки должны проходить
2. **Ревью**: минимум 1 одобрение от ревьюера
3. **Разрешение замечаний**: все замечания помечены как решенные
4. **Слияние**: выполняется принимающей стороной
5. **Удаление ветки**: после успешного слияния

### Типы слияния
- **Squash and merge**: для серии коммитов с одной задачей
- **Rebase and merge**: для чистых коммитов с линейной историей
- **Create a merge commit**: для сложных ветвлений (не рекомендуется)

## Workflow Conventions

### Создание веток
```bash
# Паттерн: <type>/<short-description>
feat/user-authentication
fix/memory-leak-cache
refactor/database-layer
```

### Процесс разработки
1. Создать ветку от `main`
2. Внести изменения атомарными коммитами
3. Открыть Pull Request в статусе Draft/WIP
4. Перевести в готовый статус после завершения
5. Пройти ревью и слияние

### Автоматизация
```yaml
# Пример .github/workflows/ci.yml
name: CI
on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Run tests
        run: pytest
        
  lint:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Check code style
        run: black --check .
      - name: Run linter
        run: flake8
```

## Best Practices

### Для разработчиков
- Перед началом работы убедитесь, что понимаете архитектуру проекта
- Обсуждайте крупные изменения до реализации
- Пишите тесты для новой функциональности
- Следуйте стилю проекта

### Для ревьюеров
- Фокусируйтесь на архитектуре и логике, а не на стиле
- Используйте конструктивную критику
- Объясняйте причины отклонений
- Уважайте время разработчиков

### Для менеджеров
- Создавайте четкие требования
- Обеспечивайте своевременную обратную связь
- Поддерживайте культуру качественного кода

---

*Эти конвенции являются рекомендательными и могут быть адаптированы под конкретные проекты.*