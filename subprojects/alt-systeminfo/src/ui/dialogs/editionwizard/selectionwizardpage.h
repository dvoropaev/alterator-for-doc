#ifndef SELECTIONWIZARDPAGE_H
#define SELECTIONWIZARDPAGE_H

#include "controller/editioncontroller.h"

#include <QItemSelectionModel>
#include <QWizardPage>

#include <memory>

namespace alt
{
namespace Ui
{
class SelectionWizardPage;
} // namespace Ui

class SelectionWizardPage : public QWizardPage
{
    Q_OBJECT

public:
    SelectionWizardPage(QWidget *parent = nullptr);
    ~SelectionWizardPage();

public:
    void setController(EditionController *controller);

private:
    bool isComplete() const override;

private slots:
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    SelectionWizardPage(const SelectionWizardPage &) = delete;
    SelectionWizardPage(SelectionWizardPage &&) = delete;
    SelectionWizardPage &operator=(const SelectionWizardPage &) = delete;
    SelectionWizardPage &operator=(SelectionWizardPage &&) = delete;

private:
    std::unique_ptr<Ui::SelectionWizardPage> m_ui;
    std::unique_ptr<QStandardItemModel> m_model;
    EditionController *m_controller;
};
} // namespace alt
#endif // SELECTIONWIZARDPAGE_H
