#include "transaction.h"

namespace alt
{
Transaction::Status Transaction::status() const
{
    return m_status;
}

Transaction::Action Transaction::action(const Component &component) const
{
    auto iter = componentActions.find(component.name);
    return iter != componentActions.end() ? iter->second : Action::Nothing;
}

Transaction::Action Transaction::action(const std::string &package) const
{
    auto iter = packageActions.find(package);
    return iter != packageActions.end() ? iter->second : Action::Nothing;
}

Transaction::SystemModuleStatus Transaction::status(const Component &component) const
{
    auto iter = componentStatuses.find(component.name);
    return iter != componentStatuses.end() ? iter->second : SystemModuleStatus{};
}

Transaction::SystemModuleStatus Transaction::status(const std::string &package) const
{
    auto iter = packageStatuses.find(package);
    return iter != packageStatuses.end() ? iter->second : SystemModuleStatus{};
}

[[nodiscard]] std::map<std::string, std::reference_wrapper<Component>> Transaction::components() const
{
    return m_components;
}

[[nodiscard]] const std::set<std::string> &Transaction::packages() const
{
    return m_packages;
}

void Transaction::setStatus(Status status)
{
    m_status = status;
}
} // namespace alt
