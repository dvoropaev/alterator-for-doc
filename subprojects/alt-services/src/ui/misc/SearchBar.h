#pragma once

#include "SearchAdapter.h"
#include <QWidget>

class SearchBar : public QWidget
{
    Q_OBJECT

public:
    explicit SearchBar(QWidget *parent = nullptr);
    ~SearchBar();

    void setAdapter(SearchAdapter* adapter);

private slots:
    void on_lineEdit_textChanged(const QString&);
    void on_prevButton_clicked();
    void on_nextButton_clicked();

private:
    class Private;
    Private* d;
};
