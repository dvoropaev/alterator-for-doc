#ifndef EDITIONCONTROLLER_H
#define EDITIONCONTROLLER_H

#include <QObject>
#include <QStandardItemModel>

namespace alt
{
class EditionController : public QObject
{
public:
    EditionController();
    ~EditionController() override;

public:
    void buildModel(QStandardItemModel *model);
    int setEdition(const QString &editionId);
    QString getLicense(const QString &editionId);
    QString getDescription(const QString &editionId);
    QString getCurrentEditionId();

    void setStateSelectedEditionId(const QString &editionId);
    QString getStateSelectedEditionId();

private:
    EditionController(const EditionController &) = delete;
    EditionController(EditionController &&) = delete;
    EditionController &operator=(const EditionController &) = delete;
    EditionController &operator=(EditionController &&) = delete;

private:
    QStandardItemModel *m_editionsModel;

    // State
    QString m_selectedEditionId;
};
} //namespace alt

#endif // MAINCONTROLLER_H
