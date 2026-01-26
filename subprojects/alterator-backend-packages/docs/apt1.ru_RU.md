[English](./apt1.md) | [Русский](./apt1.ru_RU.md)

# Интерфейс **org.altlinux.alterator.apt1**

Предоставляет команды бэкенда apt для поиска, установки, переустановки и обновления пакетов со стримингом сигналов для длительных операций.

| Метод | Описание |
|--------|---------|
| [Info](#method-Info) | Возвращает статический дескриптор объекта бэкенда apt. |
| [UpdateAsync](#method-UpdateAsync) | Выполняет apt-get update в фоне; прогресс передаётся через сигналы. |
| [ApplyAsync](#method-ApplyAsync) | Применяет транзакцию установки/удаления через apt-wrapper apply с использованием сгенерированных pkgpriorities. |
| [ReinstallAsync](#method-ReinstallAsync) | Переустанавливает пакеты асинхронно через apt-get reinstall -y -q. |
| [ListAllPackages](#method-ListAllPackages) | Перечисляет все доступные имена пакетов через apt-cache search . --names-only. |
| [Search](#method-Search) | Ищет пакеты по шаблону через apt-wrapper search (apt-cache search). |
| [LastUpdate](#method-LastUpdate) | Сообщает время последнего обновления для /var/lib/apt/lists в UTC. |
| [LastDistUpgrade](#method-LastDistUpgrade) | Возвращает последнюю запись dist-upgrade из /var/log/alterator/apt/dist-upgrades.log. |
| [CheckApply](#method-CheckApply) | Имитирует транзакцию установки/удаления и возвращает запланированные изменения. |
| [CheckReinstall](#method-CheckReinstall) | Имитирует транзакцию переустановки для выбранных пакетов. |
| [CheckDistUpgrade](#method-CheckDistUpgrade) | Имитирует dist-upgrade и сообщает запланированные установки/удаления. |
| [DistUpgradeAsync](#method-DistUpgradeAsync) | Выполняет dist-upgrade асинхронно через apt-get dist-upgrade -y -q. |


| Сигнал | Описание |
|--------|---------|
| [apt1_update_stderr_signal](#signal-apt1_update_stderr_signal) | Поток stderr от apt-get update. |
| [apt1_update_stdout_signal](#signal-apt1_update_stdout_signal) | Поток stdout от apt-get update. |
| [apt1_install_stderr_signal](#signal-apt1_install_stderr_signal) | Поток stderr от apt-wrapper apply (установка/удаление). |
| [apt1_install_stdout_signal](#signal-apt1_install_stdout_signal) | Поток stdout от apt-wrapper apply (установка/удаление). |
| [apt1_reinstall_stderr_signal](#signal-apt1_reinstall_stderr_signal) | Поток stderr от apt-get reinstall. |
| [apt1_reinstall_stdout_signal](#signal-apt1_reinstall_stdout_signal) | Поток stdout от apt-get reinstall. |
| [apt1_remove_stderr_signal](#signal-apt1_remove_stderr_signal) | Поток stderr от транзакций удаления. |
| [apt1_remove_stdout_signal](#signal-apt1_remove_stdout_signal) | Поток stdout от транзакций удаления. |
| [apt1_dist_upgrade_stderr_signal](#signal-apt1_dist_upgrade_stderr_signal) | Поток stderr от apt-get dist-upgrade. |
| [apt1_dist_upgrade_stdout_signal](#signal-apt1_dist_upgrade_stdout_signal) | Поток stdout от apt-get dist-upgrade. |

## Методы

### **Info**() -> ([stdout_bytes](#argument-stdout_bytes-of-Info) : `ay`, [response](#argument-response-of-Info) : `i`)<a id="method-Info"></a>

Возвращает статический дескриптор объекта бэкенда apt.

#### Выходные аргументы

##### **stdout_bytes** : `ay` <a id="argument-stdout_bytes-of-Info"></a>

Содержимое `/usr/share/alterator/objects/apt.object`.

TOML-описание объекта с display_name и comments.
##### **response** : `i` <a id="argument-response-of-Info"></a>

Код завершения помощника cat.

0 — успех, != 0 — ошибка.
### **UpdateAsync**() -> ([response](#argument-response-of-UpdateAsync) : `i`)<a id="method-UpdateAsync"></a>

Выполняет apt-get update в фоне; прогресс передаётся через сигналы.

Сигналы: apt1_update_stdout_signal, apt1_update_stderr_signals.
#### Выходные аргументы

##### **response** : `i` <a id="argument-response-of-UpdateAsync"></a>

Код завершения apt-get update.

0 — успех, != 0 — ошибка.
### **ApplyAsync**([exclude_pkgnames](#argument-exclude_pkgnames-of-ApplyAsync) : `s`, [pkgnames](#argument-pkgnames-of-ApplyAsync) : `s`) -> ([response](#argument-response-of-ApplyAsync) : `i`)<a id="method-ApplyAsync"></a>

Применяет транзакцию установки/удаления через apt-wrapper apply с использованием сгенерированных pkgpriorities.

Создаёт временный файл pkgpriorities (mktemp "${TMPDIR:-/tmp}/alterator-pkgpriorities.XXXXXXXXXXXX"), который перечисляет пакеты, помеченные как manual командой apt-mark showmanual; предотвращает неявное удаление этих пакетов, если они не указаны в exclude_pkgnames. Временный файл удаляется при завершении apt-wrapper.
Сигналы: apt1_install_stdout_signal, apt1_install_stderr_signal, apt1_remove_stdout_signal, apt1_remove_stderr_signal.
#### Входные аргументы

##### **exclude_pkgnames** : `s` <a id="argument-exclude_pkgnames-of-ApplyAsync"></a>

Имена пакетов через пробел, которые нужно исключить из pkgpriorities.

Этот список позволяет удалять manual-пакеты по зависимостям. При вызове через busctl передавайте литеральную строку '' для пустого списка; пустая строка сдвигает первый элемент pkgnames в exclude_pkgnames.
##### **pkgnames** : `s` <a id="argument-pkgnames-of-ApplyAsync"></a>

Имена пакетов через пробел для обработки.

Имена, заканчивающиеся на "-", удаляются; остальные устанавливаются через apt-get install -y -q.
#### Выходные аргументы

##### **response** : `i` <a id="argument-response-of-ApplyAsync"></a>

Код завершения apt-wrapper apply.

0 — успех, != 0 — ошибка.
### **ReinstallAsync**([pkgnames](#argument-pkgnames-of-ReinstallAsync) : `s`) -> ([response](#argument-response-of-ReinstallAsync) : `i`)<a id="method-ReinstallAsync"></a>

Переустанавливает пакеты асинхронно через apt-get reinstall -y -q.

#### Входные аргументы

##### **pkgnames** : `s` <a id="argument-pkgnames-of-ReinstallAsync"></a>

Имена пакетов через пробел для переустановки.

#### Выходные аргументы

##### **response** : `i` <a id="argument-response-of-ReinstallAsync"></a>

Код завершения команды переустановки.

0 — успех, != 0 — ошибка.
### **ListAllPackages**() -> ([stdout_strings](#argument-stdout_strings-of-ListAllPackages) : `as`, [stderr_strings](#argument-stderr_strings-of-ListAllPackages) : `as`, [response](#argument-response-of-ListAllPackages) : `i`)<a id="method-ListAllPackages"></a>

Перечисляет все доступные имена пакетов через apt-cache search . --names-only.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-ListAllPackages"></a>

Имена пакетов, сформированные apt-cache.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-ListAllPackages"></a>

Ошибки или предупреждения от apt-cache.

##### **response** : `i` <a id="argument-response-of-ListAllPackages"></a>

Код завершения помощника listall.

0 — успех, != 0 — ошибка.
### **Search**([pattern](#argument-pattern-of-Search) : `s`) -> ([stdout_strings](#argument-stdout_strings-of-Search) : `as`, [stderr_strings](#argument-stderr_strings-of-Search) : `as`, [response](#argument-response-of-Search) : `i`)<a id="method-Search"></a>

Ищет пакеты по шаблону через apt-wrapper search (apt-cache search).

#### Входные аргументы

##### **pattern** : `s` <a id="argument-pattern-of-Search"></a>

Выражение поиска, передаваемое в apt-cache search.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-Search"></a>

Совпадающие имена пакетов, извлечённые из вывода apt-cache.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-Search"></a>

Ошибки команды поиска.

##### **response** : `i` <a id="argument-response-of-Search"></a>

Код завершения помощника поиска.

0 — успех, != 0 — ошибка.
### **LastUpdate**() -> ([stdout_strings](#argument-stdout_strings-of-LastUpdate) : `as`, [stderr_strings](#argument-stderr_strings-of-LastUpdate) : `as`, [response](#argument-response-of-LastUpdate) : `i`)<a id="method-LastUpdate"></a>

Сообщает время последнего обновления для /var/lib/apt/lists в UTC.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-LastUpdate"></a>

Одна запись со строкой даты-времени (YYYY-MM-DD HH:MM:SS UTC).

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-LastUpdate"></a>

Ошибки при stat или разборе.

##### **response** : `i` <a id="argument-response-of-LastUpdate"></a>

Код завершения помощника метки времени.

0 — успех, != 0 — ошибка.
### **LastDistUpgrade**() -> ([stdout_strings](#argument-stdout_strings-of-LastDistUpgrade) : `as`, [stderr_strings](#argument-stderr_strings-of-LastDistUpgrade) : `as`, [response](#argument-response-of-LastDistUpgrade) : `i`)<a id="method-LastDistUpgrade"></a>

Возвращает последнюю запись dist-upgrade из /var/log/alterator/apt/dist-upgrades.log.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-LastDistUpgrade"></a>

Последняя строка с временной меткой из dist-upgrades.log.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-LastDistUpgrade"></a>

Ошибки, если журнал отсутствует или недоступен.

##### **response** : `i` <a id="argument-response-of-LastDistUpgrade"></a>

Код завершения чтения журнала.

0 — успех, != 0 — ошибка.
### **CheckApply**([pkgnames](#argument-pkgnames-of-CheckApply) : `s`) -> ([stdout_strings](#argument-stdout_strings-of-CheckApply) : `as`, [stderr_strings](#argument-stderr_strings-of-CheckApply) : `as`, [response](#argument-response-of-CheckApply) : `i`)<a id="method-CheckApply"></a>

Имитирует транзакцию установки/удаления и возвращает запланированные изменения.

#### Входные аргументы

##### **pkgnames** : `s` <a id="argument-pkgnames-of-CheckApply"></a>

Имена пакетов через пробел для имитации.

Имена, заканчивающиеся на "-", трактуются как удаления.
#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-CheckApply"></a>

Строка JSON с массивами install_packages, remove_packages, extra_remove_packages.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-CheckApply"></a>

Диагностический вывод apt-get --just-print.

##### **response** : `i` <a id="argument-response-of-CheckApply"></a>

Код завершения имитации.

0 — успех, != 0 — ошибка.
### **CheckReinstall**([pkgnames](#argument-pkgnames-of-CheckReinstall) : `s`) -> ([stdout_strings](#argument-stdout_strings-of-CheckReinstall) : `as`, [stderr_strings](#argument-stderr_strings-of-CheckReinstall) : `as`, [response](#argument-response-of-CheckReinstall) : `i`)<a id="method-CheckReinstall"></a>

Имитирует транзакцию переустановки для выбранных пакетов.

#### Входные аргументы

##### **pkgnames** : `s` <a id="argument-pkgnames-of-CheckReinstall"></a>

Имена пакетов через пробел для переустановки.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-CheckReinstall"></a>

Пакеты, запланированные к установке по выводу apt-get -s.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-CheckReinstall"></a>

Пакеты, запланированные к удалению, или предупреждения.

##### **response** : `i` <a id="argument-response-of-CheckReinstall"></a>

Код завершения имитации.

0 — успех, != 0 — ошибка.
### **CheckDistUpgrade**() -> ([stdout_strings](#argument-stdout_strings-of-CheckDistUpgrade) : `as`, [stderr_strings](#argument-stderr_strings-of-CheckDistUpgrade) : `as`, [response](#argument-response-of-CheckDistUpgrade) : `i`)<a id="method-CheckDistUpgrade"></a>

Имитирует dist-upgrade и сообщает запланированные установки/удаления.

#### Выходные аргументы

##### **stdout_strings** : `as` <a id="argument-stdout_strings-of-CheckDistUpgrade"></a>

Пакеты, запланированные к установке.

##### **stderr_strings** : `as` <a id="argument-stderr_strings-of-CheckDistUpgrade"></a>

Пакеты, запланированные к удалению, или ошибки.

##### **response** : `i` <a id="argument-response-of-CheckDistUpgrade"></a>

Код завершения apt-get dist-upgrade -s -q.

0 — успех, != 0 — ошибка.
### **DistUpgradeAsync**() -> ([response](#argument-response-of-DistUpgradeAsync) : `i`)<a id="method-DistUpgradeAsync"></a>

Выполняет dist-upgrade асинхронно через apt-get dist-upgrade -y -q.

#### Выходные аргументы

##### **response** : `i` <a id="argument-response-of-DistUpgradeAsync"></a>

Код завершения команды обновления.

0 — успех, != 0 — ошибка.
## Сигналы

### **apt1_update_stderr_signal**(`s`)<a id="signal-apt1_update_stderr_signal"></a>

Поток stderr от apt-get update.

#### Выходные аргументы

##### Argument `s`

### **apt1_update_stdout_signal**(`s`)<a id="signal-apt1_update_stdout_signal"></a>

Поток stdout от apt-get update.

#### Выходные аргументы

##### Argument `s`

### **apt1_install_stderr_signal**(`s`)<a id="signal-apt1_install_stderr_signal"></a>

Поток stderr от apt-wrapper apply (установка/удаление).

#### Выходные аргументы

##### Argument `s`

### **apt1_install_stdout_signal**(`s`)<a id="signal-apt1_install_stdout_signal"></a>

Поток stdout от apt-wrapper apply (установка/удаление).

#### Выходные аргументы

##### Argument `s`

### **apt1_reinstall_stderr_signal**(`s`)<a id="signal-apt1_reinstall_stderr_signal"></a>

Поток stderr от apt-get reinstall.

#### Выходные аргументы

##### Argument `s`

### **apt1_reinstall_stdout_signal**(`s`)<a id="signal-apt1_reinstall_stdout_signal"></a>

Поток stdout от apt-get reinstall.

#### Выходные аргументы

##### Argument `s`

### **apt1_remove_stderr_signal**(`s`)<a id="signal-apt1_remove_stderr_signal"></a>

Поток stderr от транзакций удаления.

#### Выходные аргументы

##### Argument `s`

### **apt1_remove_stdout_signal**(`s`)<a id="signal-apt1_remove_stdout_signal"></a>

Поток stdout от транзакций удаления.

#### Выходные аргументы

##### Argument `s`

### **apt1_dist_upgrade_stderr_signal**(`s`)<a id="signal-apt1_dist_upgrade_stderr_signal"></a>

Поток stderr от apt-get dist-upgrade.

#### Выходные аргументы

##### Argument `s`

### **apt1_dist_upgrade_stdout_signal**(`s`)<a id="signal-apt1_dist_upgrade_stdout_signal"></a>

Поток stdout от apt-get dist-upgrade.

#### Выходные аргументы

##### Argument `s`
