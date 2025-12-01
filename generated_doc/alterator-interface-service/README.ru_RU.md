**alterator-interface-service**

[English](README.md) | [Русский](README.ru_RU.md)

Определяет интерфейс D-Bus `org.altlinux.alterator.service1` для управления жизненным циклом сервисов.

---

# Общие сведения
| Компонент | Расположение | Назначение |
| --------- | ------------ | ---------- |
| Файл `org.altlinux.alterator.service1.xml` | `/usr/share/dbus-1/interfaces/org.altlinux.alterator.service1.xml` | Описывает данные интроспекции D-Bus для методов и сигналов управления сервисом. |
| Файл `org.altlinux.alterator.service1.policy` | `/usr/share/polkit-1/actions/org.altlinux.alterator.service1.policy` | Определяет правила PolicyKit для вызова методов `org.altlinux.alterator.service1`. |

# Возможности
- Формализует операции жизненного цикла сервисов: развертывание, настройка, запуск, остановка, резервное копирование, восстановление, отключение.
- Передаёт стандартные потоки stdout и stderr процесса через сигналы `service_stdout_signal` и `service_stderr_signal`.
- Задаёт параметры авторизации для интерактивных и автоматизированных вызовов через PolicyKit.

# Интеграция с другими компонентами
- Клиенты Alterator используют описание интерфейса из `/usr/share/dbus-1/interfaces/org.altlinux.alterator.service1.xml` для вызова методов backend-сервисов.
- Авторизация отдельных методов определяется правилами `/usr/share/polkit-1/actions/org.altlinux.alterator.service1.policy`: `Deploy`, `Backup`, `Undeploy`, `Restore` требуют `auth_admin_keep`; остальные вызовы разрешены без ограничений.
- Параметры для операций, требующих конфигурационных данных (`Deploy`, `Configure`, `Undeploy`, `Backup`, `Restore`), передаются как единая строка в `stdin` backend-процесса.

# Документация по интерфейсам
- `service1` — предоставляет методы управления жизненным циклом сервиса и получения статуса. См. [service1.md](./service1.md).
