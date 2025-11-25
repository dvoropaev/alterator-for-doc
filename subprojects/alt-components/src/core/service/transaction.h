#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "entity/component.h"

#include <chrono>
#include <map>
#include <set>

namespace alt
{
class TransactionService;
class Transaction
{
    // NOTE(sheriffkorov): required to setStatus calls
    friend TransactionService;

public:
    using TimePoint = std::chrono::system_clock::time_point;
    enum Status
    {
        New,
        Resolving,
        Denied,
        Allowed,
        Processing,
        Failed,
        Completed,
    };
    enum Action : unsigned char
    {
        Nothing,
        Install,
        Remove,
    };
    struct SystemModuleStatus;

public:
    [[nodiscard]] Status status() const;
    [[nodiscard]] SystemModuleStatus status(const Component &component) const;
    [[nodiscard]] SystemModuleStatus status(const std::string &package) const;
    [[nodiscard]] Action action(const Component &component) const;
    [[nodiscard]] Action action(const std::string &package) const;
    [[nodiscard]] std::map<std::string, std::reference_wrapper<Component>> components() const;
    [[nodiscard]] const std::set<std::string> &packages() const;

private:
    void setStatus(Status status);

private:
    std::map<std::string, std::reference_wrapper<Component>> m_components;
    std::set<std::string> m_packages;
    std::map<std::string, SystemModuleStatus> componentStatuses;
    std::map<std::string, SystemModuleStatus> packageStatuses;
    std::map<std::string, Action> componentActions;
    std::map<std::string, Action> packageActions;
    Status m_status;

public:
    TimePoint wasCreated;
};

struct Transaction::SystemModuleStatus
{
    bool isManuallyInstalled = false;
    bool isBase = false;
    std::set<std::string> relation;
};
} // namespace alt

#endif // TRANSACTION_H
