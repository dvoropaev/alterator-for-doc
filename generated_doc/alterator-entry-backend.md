# Alterator Entry и backend

## Краткие определения
- Alterator Entry — спецификация дескриптора сущности Alterator, задающая формат хранения данных о сущности и используемая для публикации объектов через D-Bus. 【F:subprojects/alterator-entry/doc/README.md†L36-L39】
- Backend — тип сущности Alterator Entry, описывающий D-Bus интерфейс, его узел, связанные параметры и набор методов. 【F:subprojects/alterator-entry/doc/README.md†L410-L436】

## Назначение и размещение файлов
- Файлы Alterator Entry именуются по шаблону `<Name>.<Type>` и получают расширение по значению ключа `Type`; для backend дополнительно указывается интерфейс. 【F:subprojects/alterator-entry/doc/README.md†L41-L63】
- Базовые пути различаются по происхождению сущности: системные описания ставятся в `/usr/share/alterator/<директория>/<имя>.<расширение>`, пользовательские — в `/etc/alterator/<директория>/<имя>.<расширение>`. 【F:subprojects/alterator-entry/doc/README.md†L78-L104】
- Backend поддерживает режимы `system` и `user`; пользовательские описания помещаются в подкаталог `backends/user/`. 【F:subprojects/alterator-entry/doc/README.md†L100-L104】

## Структура Backend-описания
- Поле `module` фиксирует обработчик для сущности; в текущей реализации применяется модуль executor. 【F:subprojects/alterator-entry/doc/README.md†L417-L418】
- Поле `interface` задаёт имя D-Bus интерфейса (полное либо с добавлением префикса `org.altlinux.alterator.`). 【F:subprojects/alterator-entry/doc/README.md†L418-L419】
- Поле `name` определяет уникальный узел (объект) в дереве `/org/altlinux/alterator/`. 【F:subprojects/alterator-entry/doc/README.md†L419-L420】
- Параметры `thread_limit` и `action_id` управляют ограничениями параллелизма и интеграцией с polkit. 【F:subprojects/alterator-entry/doc/README.md†L420-L421】
- Секция `methods` описывает доступные методы: команда запуска (`execute`), режимы возврата потоков (`stdout_strings`, `stdout_bytes`, `stderr_strings`), лимиты буферов, сигналы и ограничения по времени и потокам. 【F:subprojects/alterator-entry/doc/README.md†L422-L436】

## Взаимосвязь Alterator Entry и backend
- Alterator Entry определяет общий формат описания сущностей Alterator и поддерживает несколько типов, включая backend. 【F:subprojects/alterator-entry/doc/README.md†L69-L76】
- Файлы с типом Backend считываются модулем `alterator-module-executor`, который по описаниям создаёт и регистрирует соответствующие D-Bus интерфейсы. 【F:subprojects/alterator-entry/doc/README.md†L410-L412】
- Диагностические сущности могут ссылаться на backend через назначение `assignment = Universal`, что допускает выполнение тестов в любом контексте, определённом backend. 【F:subprojects/alterator-entry/doc/README.md†L403-L404】
- Через механизмы размещения и единую структуру Alterator Entry поддерживается согласованность между объектными описаниями, диагностическими инструментами и backend-интерфейсами. 【F:subprojects/alterator-entry/doc/README.md†L78-L104】【F:subprojects/alterator-entry/doc/README.md†L410-L436】

## Проверка корректности
- Убедитесь, что `.backend` файлы расположены по спецификации и содержат обязательные ключи (`module`, `interface`, `name`, `methods`), чтобы `alterator-module-executor` смог опубликовать интерфейс. 【F:subprojects/alterator-entry/doc/README.md†L410-L436】
