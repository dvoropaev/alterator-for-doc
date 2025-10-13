#pragma once

#include "Parameter.h"
#include "Resource.h"

#include <QAbstractItemModel>
#include <QIcon>

class ParameterModel;
class  ResourceModel;

class DiagTool;
template<typename T>
struct Togglable;

class Service : public TranslatableObject
{
public:
    Service(const QString& name,
            const Locales& locales,
            const QString& path,
            const QString& iconName,
            bool forceDeployable,
            PtrVector<Parameter>&& parameters,
            PtrVector<Resource>&& resources,
            PtrVector<DiagTool>&& tools, bool diagMissing);
    ~Service();

    ParameterModel* parameterModel();
     ResourceModel*  resourceModel();

    void setLocale(const QLocale& locale) const override;

    inline const QString& dbusPath(){ return m_dbusPath; }

    const QIcon& icon();

    inline bool isDiagMissing() {return m_diagNotFound;}

    inline bool isForceDeployable(){return m_force_deployable;}
    inline bool forceDeploy(){return m_force_deploy;}
    inline void setForceDeploy(bool how){m_force_deploy = how;}

    inline bool isDeployed(){return m_deployed;}
    inline bool isStarted(){return m_started;}

    const PtrVector<Parameter>& parameters();
    const PtrVector<DiagTool>& diagTools();

    QJsonObject getParameters(Parameter::Contexts ctx, bool excludePasswords = false);

    void showDefault(bool how = true);

    bool hasConflict(Service* another, Resource** ours, Resource** theirs);

    void setStatus(int code, const QByteArray& data);
    bool tryFill(QJsonObject o, Parameter::Contexts ctx);

private:
    QString m_dbusPath;
    const bool m_force_deployable;
    bool m_force_deploy{false};
    bool m_deployed{false};
    bool m_started{false};
    bool m_diagNotFound{false};

    class Private;
    Private* d;
};

class DiagTool : public TranslatableObject {
public:
    class Test : public TranslatableObject{
    public:

        enum Mode {
            PreDeploy          = 0b000001,
            PreDeploySelected  = 0b000010,
            PreDeployRequired  = 0b000110,
            PostDeploy         = 0b001000,
            PostDeploySelected = 0b010000,
            PostDeployRequired = 0b110000,
        };
        Q_DECLARE_FLAGS(Modes, Mode);

        inline Test(const QString& name, const Locales& locales, const QString& iconName, Modes flags)
            : TranslatableObject{name, locales}
            , m_modes{flags}
            , m_icon{QIcon::fromTheme(iconName)}
        {}

        inline bool isRequired(Mode m) const {
            return m_modes.testFlag( m == PreDeploy ? Mode::PreDeployRequired : Mode::PostDeployRequired );
        }

        inline bool isEnabled(Mode m) const {
            return m_modes.testFlag( m == PreDeploy ? Mode::PreDeploySelected : Mode::PostDeploySelected );
        }

        inline void setSelected(Mode m, bool selected) const {
            if ( !isRequired(m) )
                m_modes.setFlag( m == PreDeploy ? Mode::PreDeploySelected : Mode::PostDeploySelected, selected );
        }
        inline Modes modes() const { return m_modes; }

        inline const QIcon& icon() { return m_icon; }

        private:
        mutable Modes m_modes;
        const QIcon m_icon;
    };

    class Parameter : public TranslatableObject {
    public:
        inline Parameter(const QString& name, const Locales& locales, Property::Type type)
            : TranslatableObject{name, locales}
            , m_type{type}
        {}

        inline Property::Type type() const { return m_type; }

    private:
        Property::Type m_type;
    };

    inline DiagTool(const QString& name,
             const Locales& locales,
             const QString& path,
             const QString& iconName,
             bool session,
             PtrVector<Test>&& tests,
             PtrVector<Parameter>&& parameters)
        : TranslatableObject{name,locales}
        , m_session{session}
        , m_path{path}
        , m_parameters{std::move(parameters)}
        , m_tests{std::move(tests)}
        , m_icon{QIcon::fromTheme(iconName)}
    {}

    inline const auto& parameters() const {return m_parameters;}
    inline auto& tests() {return m_tests;}
    inline const auto& path() const { return m_path; }
    inline bool session() const {return m_session;}
    inline bool hasTests(Test::Mode m) {
        return std::any_of(m_tests.cbegin(), m_tests.cend(),
                           [m](const auto& test){return test->modes().testFlag(m);});
    }
    inline bool anySelected(Test::Mode m) {
        return std::any_of(m_tests.cbegin(), m_tests.cend(), [m](const auto& test){return test->isEnabled(m);});
    }

    inline bool isMissingParams(){ return m_params_missing; }

    void setLocale(const QLocale& locale) const override {
        for ( auto& param : m_parameters )
            param->setLocale(locale);
        for ( auto& test : m_tests )
            test->setLocale(locale);
        TranslatableObject::setLocale(locale);
    }

    inline const QIcon& icon() const { return m_icon; }

protected:
    bool m_params_missing{false};
    friend class Service;

private:
    const bool m_session;
    const QString m_path;
    PtrVector<Test> m_tests;
    const PtrVector<Parameter> m_parameters;
    const QIcon m_icon;
};
