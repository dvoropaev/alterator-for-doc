#pragma once

#include <QObject>

/*
 * Search adapter interface.
 */
class SearchAdapter : public QObject {
    Q_OBJECT

public:
    virtual ~SearchAdapter() = default;
    
    /*
     * Performs search of input string
     * Returns count of matches
     */
    virtual int search(const QString&) = 0;
    
public slots:
    virtual void prev() = 0;
    virtual void next() = 0;
};
