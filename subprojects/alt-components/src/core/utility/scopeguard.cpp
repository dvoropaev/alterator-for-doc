#include "scopeguard.h"

namespace alt
{
ScopeGuard::ScopeGuard(const std::function<void()> &on_destroy)
    : on_destroy(on_destroy)
    , active(true)
{}

ScopeGuard::~ScopeGuard()
{
    if (active && on_destroy)
    {
        on_destroy();
    }
}

void ScopeGuard::dismiss() noexcept
{
    active = false;
}

void ScopeGuard::activate() noexcept
{
    active = true;
}
} // namespace alt
