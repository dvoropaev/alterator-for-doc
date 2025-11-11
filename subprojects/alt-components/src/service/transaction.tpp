#ifndef TRANSACTION_TPP
#define TRANSACTION_TPP

#ifndef TRANSACTION_H
#error __FILE__ should only be included from transaction.h.
#endif // TRANSACTION_H

#include <map>
#include <set>
#include <QString>

namespace alt
{
template<typename T>
std::set<QString> Transaction::toSet(const std::map<QString, T> &map)
{
    std::set<QString> result;
    for (const auto &[str, _] : map)
    {
        result.insert(str);
    }
    return result;
}
} // namespace alt

#endif // TRANSACTION_TPP
