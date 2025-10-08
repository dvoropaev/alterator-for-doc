#ifndef EDITIONFIELDWIDGET_H
#define EDITIONFIELDWIDGET_H

#include <QItemSelectionModel>
#include <QWidget>

#include <memory>

namespace alt
{
namespace Ui
{
class EditionFieldWidget;
} // namespace Ui

class EditionFieldWidget : public QWidget
{
    Q_OBJECT

public:
    EditionFieldWidget(QWidget *parent = nullptr);
    ~EditionFieldWidget();

signals:
    void requestEdit();

public:
    void setText(const QString &text);

private:
    EditionFieldWidget(const EditionFieldWidget &) = delete;
    EditionFieldWidget(EditionFieldWidget &&) = delete;
    EditionFieldWidget &operator=(const EditionFieldWidget &) = delete;
    EditionFieldWidget &operator=(EditionFieldWidget &&) = delete;

private:
    std::unique_ptr<Ui::EditionFieldWidget> m_ui;
};
} // namespace alt
#endif // EDITIONFIELDWIDGET_H
