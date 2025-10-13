#ifndef PROGRESS_WIZARDPAGE_H
#define PROGRESS_WIZARDPAGE_H

#include "wizardpage.h"

#include <memory>
#include <QTextBrowser>

namespace alt
{
class ProgressWizardPage : public WizardPage
{
    Q_OBJECT

public:
    void initialize() override;

public:
    explicit ProgressWizardPage(QWidget *parent = nullptr);
    ~ProgressWizardPage() override;

public:
    ProgressWizardPage(const ProgressWizardPage &) = delete;
    ProgressWizardPage(ProgressWizardPage &&) = delete;
    ProgressWizardPage &operator=(const ProgressWizardPage &) = delete;
    ProgressWizardPage &operator=(ProgressWizardPage &&) = delete;

public slots:
    void onNewLine(const QString &line);

private:
    std::unique_ptr<QTextBrowser> textBrowser;
};
} // namespace alt
#endif // PROGRESS_WIZARDPAGE_H
