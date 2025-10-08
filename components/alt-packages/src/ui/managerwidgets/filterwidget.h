#ifndef FILTERWIDGET_H
#define FILTERWIDGET_H

#include <QWidget>

namespace Ui
{
class FilterWidget;
}

class FilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FilterWidget(QWidget *parent = nullptr);
    ~FilterWidget();

public:
    void setName(const QString &text);
    void setOptions(const QStringList &options);
    bool all(const QString &text);

public slots:
    void changeEvent(QEvent *event) override;

signals:
    void filterChanged(const QString &text);

private slots:
    void onCurrentTextChanged(const QString &text);

private:
    FilterWidget(const FilterWidget &)            = delete;
    FilterWidget(FilterWidget &&)                 = delete;
    FilterWidget &operator=(const FilterWidget &) = delete;
    FilterWidget &operator=(FilterWidget &&)      = delete;

protected:
    std::unique_ptr<Ui::FilterWidget> m_ui;
};

#endif // FILTERWIDGET_H
