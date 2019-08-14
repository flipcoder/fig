#include "FigApp.h"
#include "FigWindow.h"
#include <vector>
#include "kit/kit.h"
#include "kit/log/log.h"
#include <QDialogButtonBox>
using namespace std;
using kit::make_unique;

FigApp :: FigApp(int& argc, char* argv[]):
    QApplication(argc,argv),
    m_Args(argc, (const char**)argv),
    m_FigAppPath(argv[0])
{
    if(m_Args.size()>0)
        m_SettingsFn = m_Args.get(m_Args.size()-1);
    else
        m_SettingsFn = "settings.json";
    
    m_SchemaFn = m_Args.value_or("schema", "settings.schema.json");
    
    m_Title = m_Args.value_or("title", "Settings");

    try{
        m_pSettings = make_shared<Meta>(m_SettingsFn);
    }catch(Error& e){
        QMessageBox::critical(m_pWindow.get(), "Error", "Could not load settings file.");
        quit();
        goto return_ctor;
    }
    
    try{
        m_pSchema = make_shared<Meta>(m_SchemaFn);
        //m_pSchema = make_shared<Schema>(make_shared<Meta>(m_SchemaFn));
    }catch(Error& e){
        QMessageBox::critical(m_pWindow.get(), "Error", "Could not load schema file.");
        quit();
        goto return_ctor;
    }
 
    init();
    m_Success = true;

    return_ctor: {}
}

FigApp :: ~FigApp()
{
}

string FigApp :: as_string(std::shared_ptr<Meta> m, string key)
{
    string r;
    try{
        r = m->at<string>(key);
    }catch(...){
        try{
            r = to_string(m->at<int>(key));
        }catch(...){
            try{
                r = to_string(m->at<double>(key));
            }catch(...){
                try{
                    r = m->at<bool>(key) ? "true" : "false";
                }catch(...){}
            }
        }
    }
    return r;
}

string FigApp :: as_string(const MetaElement& me)
{
    string r;
    try{
        r = me.as<string>();
    }catch(...){
        try{
            r = to_string(me.as<int>());
        }catch(...){
            try{
                r = to_string(me.as<double>());
            }catch(...){
                try{
                    r = me.as<bool>() ? "true" : "false";
                }catch(...){}
            }
        }
    }
    return r;

}

void FigApp :: init()
{
    m_pWindow = make_unique<FigWindow>(this);
    
    //QPushButton *button1 = new QPushButton("One");
    //QPushButton *button2 = new QPushButton("Two");
    QVBoxLayout* layout = new QVBoxLayout;
    //auto widget = new QWidget();
    //layout->addWidget(button1);
    //layout->addWidget(button2);
    //m_pWindow->setLayout(layout);
    //m_pWindow->setCentralWidget(widget);

    if(m_pSchema->has(".icon")){
        auto icon = m_pSchema->at<string>(".icon");
        QImage image;
        image.load(icon.c_str());
        auto label = new QLabel();
        auto pm = QPixmap::fromImage(image);
        pm = pm.scaled(QSize(
            m_pSchema->at<int>(".icon-width",128),
            m_pSchema->at<int>(".icon-height",128)
        ), Qt::KeepAspectRatio);
        label->setPixmap(pm);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
    }
    
    if(m_pSchema->has(".header")){
        auto header = m_pSchema->at<string>(".header");
        auto label = new QLabel(header.c_str());
        label->setTextFormat(Qt::RichText);
        label->setTextInteractionFlags(Qt::TextBrowserInteraction);
        label->setOpenExternalLinks(true);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
    }

    for(auto&& category: *m_pSchema)
    {
        shared_ptr<Meta> sp;
        try{
            sp = category.as<shared_ptr<Meta>>();
        }catch(...){continue;} // not a category
        if(sp->empty())
            continue;
        if(not sp->at<bool>(".visible", true))
            continue; // category not visible
        string category_name;
        try{
            category_name = sp->at<string>(".name");
        }catch(...){
            category_name = category.key;
        }
        auto group = new QGroupBox(category_name.c_str());
        auto group_layout = new QFormLayout;
        for(auto&& option: *sp){
            shared_ptr<Meta> op;
            try {
                op = option.as<shared_ptr<Meta>>();
            }catch(...){continue;} // not an option
            if(not op->at<bool>(".visible", true))
                continue; // option not visible
            string option_name;
            try {
                option_name = op->at<string>(".name");
            }catch(...){continue;} // no name
            auto box = new QComboBox;
            shared_ptr<Meta> ops;
            try{
                ops = op->meta(".options");
            }catch(...){} // no options? leave null
            shared_ptr<Meta> vals;
            try{
                vals = op->meta(".values");
            }catch(...){} // no values? leave null
            string def = as_string(op, ".default");

            // check if values are bools
            bool bools = false;
            if(vals)
            {
                int i;
                int vsz = vals->size();
                for(i=0;i<vsz;++i){
                    try{
                        vals->at<bool>(i);
                    }catch(...){break;} // not a bool, fail
                    if(i==vsz-1)
                       bools = true; // all values are bools
                }
            }

            if(bools)
            {
                group_layout->addRow(new QLabel(option_name.c_str()), new QCheckBox);
            }
            else if(vals)
            {
                // if values exist, use a combobox
                int j=0;
                for(auto&& val: *vals){
                    string v = as_string(val);
                    if(not v.empty()){
                        QVariant qj = j;
                        if(ops)
                        {
                            try{
                                box->addItem(tr(ops->at<string>(j).c_str()), qj);
                            }catch(...){
                                box->addItem(tr(v.c_str()), qj);
                            }
                        }
                        else
                        {
                            box->addItem(tr(v.c_str()), qj);
                        }
                    }
                    
                    ++j;
                }
                group_layout->addRow(new QLabel(option_name.c_str()), box);
            }
            else if(op->has(".max"))
            {
                // slider
                string suffix;
                try{
                    suffix = op->at<string>(".suffix");
                }catch(...){}
                if(not suffix.empty())
                    option_name += " ("+suffix+")";
                auto slider = new QSlider(Qt::Horizontal);
                group_layout->addRow(new QLabel((option_name).c_str()), slider);
            }
            else
            {
                //string def;
                //try{
                //    def = op->at<string>(".default");
                //}catch(...){
                //}
                // field
                group_layout->addRow(
                    new QLabel((option_name).c_str()),
                    new QLineEdit(def.c_str())
                );
            }
        }
        group->setLayout(group_layout);
        layout->addWidget(group);
    }
    
    if(m_pSchema->has(".footer")){
        auto footer = m_pSchema->at<string>(".footer");
        auto label = new QLabel(footer.c_str());
        label->setTextFormat(Qt::RichText);
        label->setTextInteractionFlags(Qt::TextBrowserInteraction);
        label->setOpenExternalLinks(true);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
    }
    
    auto okcancel = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    layout->addWidget(okcancel);
    connect(okcancel->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(restore_defaults()));
    connect(okcancel->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(save_and_quit()));
    connect(okcancel->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(quit()));
    
    m_pWindow->setLayout(layout);
    m_pWindow->setWindowTitle(m_Title.c_str());
    m_pWindow->show();
}

bool FigApp :: event(QEvent* ev)
{
    return QApplication::event(ev);
}

void FigApp :: restore_defaults()
{
    LOG("restore_defaults");
}

void FigApp :: save_and_quit()
{
    // TODO: save
    LOG("save_and_quit");
    QApplication::quit();
}
    
void FigApp :: quit()
{
    QApplication::quit();
}

