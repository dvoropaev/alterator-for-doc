#ifndef SCOPE_GUARD_H
#define SCOPE_GUARD_H

#include <functional>

namespace alt
{
class ScopeGuard
{
public:
    explicit ScopeGuard(const std::function<void()> &on_destroy);
    ~ScopeGuard();
    ScopeGuard(const ScopeGuard &) = delete;
    ScopeGuard(ScopeGuard &&) = delete;
    ScopeGuard &operator=(const ScopeGuard &) = delete;
    ScopeGuard &operator=(ScopeGuard &&) = delete;

public:
    void dismiss() noexcept;
    void activate() noexcept;

private:
    std::function<void()> on_destroy;
    bool active;
};
} // namespace alt
#endif // SCOPE_GUARD_H
