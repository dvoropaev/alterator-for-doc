#ifndef AB_CONTROLLER_H
#define AB_CONTROLLER_H

#include "../aobuilder/builders/aobuilderinterface.h"
#include "../aobuilder/datasource/datasourceinterface.h"
#include "model/model.h"

#include <memory>

#include <QObject>
#include <QPushButton>
#include <QStandardItemModel>

namespace ab
{
class CategoryWidget;
class MainWindow;

class ControllerPrivate;

class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(std::shared_ptr<MainWindow>,
                        std::unique_ptr<model::Model>,
                        std::unique_ptr<ao_builder::DataSourceInterface>,
                        std::unique_ptr<ao_builder::AOBuilderInterface>,
                        QObject *parent = nullptr);
    ~Controller() override;

    constexpr static const auto legacy_app{"acc-legacy"};

public:
    Controller(const Controller &) = delete;
    Controller(Controller &&) = delete;
    Controller &operator=(const Controller &) = delete;
    Controller &operator=(Controller &&) = delete;

    model::Model* model();

signals:
    void modelAboutToReload();
    void modelReloaded();

public slots:
    void moduleClicked(ao_builder::Object *obj);
    void switchBack();
    void buildModel();

private:
    void translateModel();

private:
    ControllerPrivate *d;
};
} // namespace ab

#endif // AB_CONTROLLER_H
