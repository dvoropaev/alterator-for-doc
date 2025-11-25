#ifndef TRANSACTION_SERVICE_H
#define TRANSACTION_SERVICE_H

#include "transaction.h"
#include <functional>
#include <tl/expected.hpp>

namespace alt
{
class PackageRepository;
class ComponentRepository;
class EditionRepository;
class TransactionService
{
public:
    using Callback = std::function<void(const std::string &chunk)>;
    class SafeMode;
    struct Error;

public:
    Transaction &create();
    Transaction &current();
    SafeMode safeMode();
    void setSafeMode(SafeMode mode);
    bool add(Transaction &transaction, std::reference_wrapper<Component> component);
    void discard(Transaction &transaction, std::reference_wrapper<Component> component);
    [[nodiscard]] tl::expected<std::reference_wrapper<Transaction>, TransactionService::Error> resolve(
        Transaction &transaction);
    tl::expected<void, TransactionService::Error> run(Transaction &transaction, const Callback &callback = nullptr);

public:
    TransactionService(const std::shared_ptr<PackageRepository> &packages,
                       const std::shared_ptr<ComponentRepository> &components,
                       const std::shared_ptr<EditionRepository> &editions);
    TransactionService(const TransactionService &) = delete;
    TransactionService(TransactionService &&) = delete;
    TransactionService &operator=(const TransactionService &) = delete;
    TransactionService &operator=(TransactionService &&) = delete;
    ~TransactionService();

private:
    class Private;
    std::unique_ptr<Private> p;
};

class TransactionService::SafeMode
{
public:
    enum Target : unsigned char
    {
        No = 0,
        Manually = 1,
        Base = 2,
        All = 3,
    } value;
    void set(Target variant);
    void reset(Target variant);
    bool test(Target variant) const;
    operator Target() const;
};

struct TransactionService::Error
{
    enum Type
    {
        InvalidRequest,
        AlreadyInProgress,
        NotResolved,
        Denied,
        SourceUnavailable,
        ServerOperationFailed,
    } type;
    std::string details;
};
} // namespace alt
#endif // TRANSACTION_SERVICE_H
