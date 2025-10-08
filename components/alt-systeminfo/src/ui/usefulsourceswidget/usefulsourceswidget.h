#ifndef USEFULSOURCESWIDGET_H
#define USEFULSOURCESWIDGET_H

#include <QWidget>

#include <memory>

namespace alt
{
namespace Ui
{
class UsefulSourcesWidget;
} // namespace Ui

class UsefulSourcesWidget : public QWidget
{
    Q_OBJECT

public:
    UsefulSourcesWidget(QWidget *parent = nullptr);
    ~UsefulSourcesWidget();

public:
    void update();

private:
    void showEvent(QShowEvent *) override;

private:
    UsefulSourcesWidget(const UsefulSourcesWidget &) = delete;
    UsefulSourcesWidget(UsefulSourcesWidget &&) = delete;
    UsefulSourcesWidget &operator=(const UsefulSourcesWidget &) = delete;
    UsefulSourcesWidget &operator=(UsefulSourcesWidget &&) = delete;

private:
    std::unique_ptr<Ui::UsefulSourcesWidget> m_ui;
};
} // namespace alt
#endif // USEFULSOURCESWIDGET_H
