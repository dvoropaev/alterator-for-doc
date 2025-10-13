#ifndef TRANSACTION_SERVICE_H
#define TRANSACTION_SERVICE_H

#include "transaction.h"

namespace alt
{
class TransactionService : public QObject
{
public:
    Q_OBJECT

public:
    class SafeMode
    {
    public:
        enum Target : uchar
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

public:
    static TransactionService &instance();
    static Transaction &create();
    static Transaction &current();
    static SafeMode safeMode();
    static void setSafeMode(SafeMode mode);

public slots:
    void onAptNewLine(const QString &line);

public:
    explicit TransactionService();
    ~TransactionService() override;

public:
    TransactionService(const TransactionService &) = delete;
    TransactionService(TransactionService &&) = delete;
    TransactionService &operator=(const TransactionService &) = delete;
    TransactionService &operator=(TransactionService &&) = delete;

signals:
    void aptNewLine(const QString &line);

private:
    static Transaction m_current;
    static SafeMode m_safeMode;
};
} // namespace alt
#endif // TRANSACTION_SERVICE_H
