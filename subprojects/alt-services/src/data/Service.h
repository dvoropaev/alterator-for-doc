#pragma once

#include "Parameter.h"
#include "Resource.h"

#include <QAbstractItemModel>
#include <QIcon>

class DiagTool;

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

    void setLocale(const QLocale& locale) const override;

    inline const QString& dbusPath(){ return m_dbusPath; }

    const QIcon& icon();

    inline bool isDiagMissing() {return m_diagNotFound;}
    inline bool isForceDeployable() const {return m_force_deployable;}
    inline bool isDeployed() const {return m_deployed;}
    inline bool isStarted() const {return m_started;}

    const PtrVector<Parameter>& parameters();
    const PtrVector<Resource>& resources();
    const PtrVector<DiagTool>& diagTools();

    QJsonObject getParameters(Parameter::Contexts ctx, bool excludePasswords = false);

    bool hasConflict(Service* another, Resource* theirs, Resource** ours);

    bool tryFill(QJsonObject o, Parameter::Contexts ctx);

    bool hasPreDiag() const;
    bool hasPostDiag() const;

    int statusCode() const;

protected:
    friend class Controller;
    void setStatus(int code, const QByteArray& data);
private:
    QString m_dbusPath;
    const bool m_force_deployable;
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
        enum class Mode {
            PreDeploy  = 1,
            PostDeploy = 2,

            None = 0,
            All  = PreDeploy | PostDeploy,
        };
        Q_DECLARE_FLAGS(Modes, Mode);

        inline Test(const QString& name, const Locales& locales, const QString& iconName, Modes modes, Modes required)
            : TranslatableObject{name, locales}
            , m_modes{modes}
            , m_required{required}
            , m_icon{QIcon::fromTheme(iconName)}
        {}

        inline DiagTool* tool() { return m_tool; }

        inline const Modes& mode()     const { return m_modes;    }
        inline const Modes& required() const { return m_required; }

        inline const QIcon& icon()     const { return m_icon;     }

    protected:
        friend class DiagTool;
        inline void setTool(DiagTool* parent) { m_tool = parent; }

    private:
        const Modes m_modes;
        const Modes m_required;
        const QIcon m_icon;
        DiagTool* m_tool{};
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
    {
        for ( const auto& test : m_tests )
            test->setTool(this);
    }

    inline const auto& parameters() const { return m_parameters; }
    inline const auto& tests()      const { return m_tests;      }
    inline const auto& path()       const { return m_path;       }
    inline bool session()           const { return m_session;    }

    inline bool hasTests(Test::Mode m) const {
        return ranges::any_of(m_tests,
            [m](const auto& test){return test->mode().testFlag(m);}
        );
    }

    inline bool hasRequiredTests(Test::Mode m) const {
        return ranges::any_of(m_tests,
            [m](const auto& test){return test->required().testFlag(m);}
        );
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
    const PtrVector<Test> m_tests;
    const PtrVector<Parameter> m_parameters;
    const QIcon m_icon;
};
Q_DECLARE_METATYPE(DiagTool::Test::Modes);
