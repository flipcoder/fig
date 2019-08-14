#ifndef APP_H_KS24XFVM
#define APP_H_KS24XFVM

#include <QtGui>
#include <QtWidgets>
#include <memory>
#include <QSystemTrayIcon>
#include "FigWindow.h"
#include "kit/args/args.h"
#include "kit/meta/meta.h"
#include "kit/meta/schema.h"

class FigApp:
    public QApplication
{
    Q_OBJECT
    
    public:

        FigApp(int& argc, char* argv[]);

        FigApp(const FigApp&) = default;
        FigApp(FigApp&&) = default;
        FigApp& operator=(const FigApp&) = default;
        FigApp& operator=(FigApp&&) = default;

        virtual ~FigApp();
        
        bool event(QEvent* event) override;
        bool failed() const { return not m_Success; }
        void init();

        static std::string as_string(std::shared_ptr<Meta> m, std::string key);
        static std::string as_string(const MetaElement& me);

    private Q_SLOTS:
        
        void quit();
        void save_and_quit();
        void restore_defaults();
        
    private:

        Args m_Args;
        
        std::string m_FigAppPath;
        
        std::string m_SettingsFn;
        std::string m_SchemaFn;
        std::shared_ptr<Meta> m_pSettings;
        std::shared_ptr<Meta> m_pSchema;
        std::string m_Title;
        
        std::unique_ptr<FigWindow> m_pWindow;

        bool m_Success = false;
};

#endif

