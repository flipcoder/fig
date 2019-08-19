#ifndef APP_H_KS24XFVM
#define APP_H_KS24XFVM

#include <QtGui>
#include <QtWidgets>
#include <memory>
#include <map>
#include <QSystemTrayIcon>
#include "FigWindow.h"
#include "kit/args/args.h"
#include "kit/meta/meta.h"

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
        void return_code(int r) { m_ReturnCode=r; } // called from FigApp on window close
        int return_code() const { return m_ReturnCode; }
        bool failed() const { return m_ReturnCode != 0; }
        bool init();
        void fail();

        static std::string as_string(std::shared_ptr<Meta> m, std::string key);
        static std::string as_string(const MetaElement& me);
        bool save();
        bool load();
        
        template<class T>
        int index_of_meta(std::shared_ptr<Meta>& m, T v);

        enum eReturnCodes {
            RC_SUCCESS = 0,
            RC_CLOSE = 1,
            RC_ERROR = 2
        };

    private Q_SLOTS:
        
        void quit();
        void save_and_quit();
        void restore_defaults();
        
    private:

        // category -> option -> label
        std::map<std::string, std::map<std::string, QWidget*>> m_WidgetMap;
        
        Args m_Args;
        
        std::string m_FigAppPath;
        
        std::string m_SettingsFn = "settings.json";
        std::string m_SchemaFn = "settings.schema.json";
        std::shared_ptr<Meta> m_pSettings;
        std::shared_ptr<Meta> m_pSchema;
        std::string m_Title;
        
        std::unique_ptr<FigWindow> m_pWindow;

        // this return code is both the ctor result (checked in main)
        // and the final app return code
        int m_ReturnCode = RC_ERROR; // start on error, since ctor can fail
};

#endif

