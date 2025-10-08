#ifndef EDITIONWIZARD_H
#define EDITIONWIZARD_H

#include "controller/editioncontroller.h"

#include <QItemSelectionModel>
#include <QShortcut>
#include <QWizard>

#include <memory>

namespace alt
{
namespace Ui
{
class EditionWizard;
} // namespace Ui

class EditionWizard : public QWizard
{
    Q_OBJECT

public:
    EditionWizard(QWidget *parent = nullptr);
    ~EditionWizard();

private slots:
    void done(int result) override;

private:
    EditionWizard(const EditionWizard &) = delete;
    EditionWizard(EditionWizard &&) = delete;
    EditionWizard &operator=(const EditionWizard &) = delete;
    EditionWizard &operator=(EditionWizard &&) = delete;

private:
    std::unique_ptr<Ui::EditionWizard> m_ui;
    std::unique_ptr<EditionController> m_controller;
    QShortcut m_quitShortcut;
    QShortcut m_nextShortcut;
    QShortcut m_backShortcut;
};
} // namespace alt
#endif // EDITIONWIZARD_H
