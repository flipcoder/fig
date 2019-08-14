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

    for(auto&& category: *m_pSchema)
    {
        shared_ptr<Meta> sp;
        try{
            sp = category.as<shared_ptr<Meta>>();
        }catch(...){continue;} // not a category
        if(sp->empty())
            continue;
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
            int i=0;
            if(vals)
            {
                // if values exist, use a combobox
                for(auto&& val: *vals){
                    string v;
                    try{
                        v = val.as<string>();
                    }catch(...){
                        try{
                            v = to_string(val.as<int>());
                        }catch(...){
                            try{
                                v = to_string(val.as<double>());
                            }catch(...){
                                try{
                                    v = val.as<bool>() ? "true" : "false";
                                }catch(...){
                                }
                            }
                        }
                    }
                    if(not v.empty()){
                        QVariant qi = i;
                        if(ops)
                        {
                            try{
                                box->addItem(tr(ops->at<string>(i).c_str()), qi);
                            }catch(...){
                                box->addItem(tr(v.c_str()), qi);
                            }
                        }
                        else
                        {
                            box->addItem(tr(v.c_str()), qi);
                        }
                    }
                    
                    ++i;
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
                // field
            }
        }
        group->setLayout(group_layout);
        layout->addWidget(group);
    }
    
    layout->addWidget(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel));
    
    m_pWindow->setLayout(layout);
    m_pWindow->setWindowTitle(m_Title.c_str());
    m_pWindow->show();
}

bool FigApp :: event(QEvent* ev)
{
    return QApplication::event(ev);
}

void FigApp :: quit()
{
    QApplication::quit();
}

