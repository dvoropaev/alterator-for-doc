#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>

namespace alt
{
class Application : public QApplication
{
public:
    explicit Application(int &argc, char **argv);
    ~Application() override;

    int exec();

    static QLocale getLocale();
    static void setLocale(const QLocale &locale);

public:
    Application(const Application &) = delete;
    Application(Application &&) = delete;
    Application &operator=(const Application &) = delete;
    Application &operator=(Application &&) = delete;

private:
    static void setupTranslator();

private:
    inline static QLocale currentLocale = QLocale::system();
};
} // namespace alt

#endif // APPLICATION_H
