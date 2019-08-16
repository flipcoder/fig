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
    //layout->setObjectName("layout");
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
        
        m_WidgetMap[category.key] = map<string,QWidget*>();
        auto& option_map = m_WidgetMap[category.key];
        
        auto group = new QGroupBox(category_name.c_str());
        //group->setObjectName(category.key.c_str());
        
        auto group_layout = new QFormLayout;
        group_layout->setObjectName(category.key.c_str());
        for(auto&& option: *sp){
            shared_ptr<Meta> op;
            try {
                op = option.as<shared_ptr<Meta>>();
            }catch(...){continue;} // not an option
            if(not op->at<bool>(".visible", true))
                continue; // option not visible
            string option_name = option.key;
            try {
                option_name = op->at<string>(".name");
            }catch(...){}
            shared_ptr<Meta> ops;
            try{
                ops = op->meta(".options");
            }catch(...){} // no options? leave null
            shared_ptr<Meta> vals;
            try{
                vals = op->meta(".values");
            }catch(...){} // no values? leave null
            string def = as_string(op, ".default");
            MetaType::ID typid = MetaType::ID::STRING;
            try {
                typid = op->type_id(".default");
            }catch(...){}
            string typ;
            if(typid==MetaType::ID::STRING)
                typ="string";
            else if(typid==MetaType::ID::INT)
                typ="int";
            else if(typid==MetaType::ID::REAL)
                typ="double";
            else if(typid==MetaType::ID::BOOL)
                typ="bool";

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
                auto cb = new QCheckBox;
                auto label = new QLabel(option_name.c_str());
                cb->setObjectName(option.key.c_str());
                cb->setProperty("type", typ.c_str());
                option_map[option.key] = cb;
                group_layout->addRow(label, cb);
            }
            else if(vals)
            {
                auto box = new QComboBox;
                box->setObjectName("box");
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
                auto label = new QLabel(option_name.c_str());
                box->setObjectName(option.key.c_str());
                box->setProperty("type", typ.c_str());
                option_map[option.key] = box;
                group_layout->addRow(label, box);
            }
            else if(op->has(".range"))
            {
                // slider
                string suffix;
                try{
                    suffix = op->at<string>(".suffix");
                }catch(...){}
                if(not suffix.empty())
                    option_name += " ("+suffix+")";
                auto slider = new QSlider(Qt::Horizontal);
                slider->setTickPosition(QSlider::TicksBothSides);
                auto range = op->meta(".range");
                slider->setMinimum(range->at<int>(0));
                slider->setMaximum(range->at<int>(1));
                if(op->has(".step")){
                    try{
                        slider->setSingleStep(op->at<int>(".step"));
                        try{
                            slider->setTickInterval(op->at<int>(".interval"));
                        }catch(...){
                            slider->setTickInterval(op->at<int>(".step"));
                        }
                    }catch(...){
                    }
                }
                slider->setValue(std::stoi(def));
                auto label = new QLabel(option_name.c_str());
                slider->setObjectName(option.key.c_str());
                slider->setProperty("type", typ.c_str());
                option_map[option.key] = slider;
                group_layout->addRow(label, slider);
            }
            else
            {
                //string def;
                //try{
                //    def = op->at<string>(".default");
                //}catch(...){
                //}
                // field
                auto le = new QLineEdit(def.c_str());
                auto label = new QLabel(option_name.c_str());
                le->setObjectName(option.key.c_str());
                le->setProperty("type", typ.c_str());
                option_map[option.key] = le;
                group_layout->addRow(
                    label,
                    le
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
    connect(okcancel->button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked()), this, SLOT(restore_defaults()));
    connect(okcancel->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(save_and_quit()));
    connect(okcancel->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(quit()));
    
    m_pWindow->setLayout(layout);
    m_pWindow->setWindowTitle(m_Title.c_str());

    load();
    
    m_pWindow->show();
}

void FigApp :: save()
{
    
}

void FigApp :: load()
{
    LOG("load");
    //for(auto&& c: m_WidgetMap){
    //    LOG(c.first);
    //    for(auto&& o: c.second)
    //    {
    //        LOGf("%s", o.first);
    //        LOGf("%s", o.second);
    //        o.second->buddy();
    //    }
    //}
}

bool FigApp :: event(QEvent* ev)
{
    return QApplication::event(ev);
}

void FigApp :: restore_defaults()
{
    for(auto&& category: *m_pSchema)
    {
        shared_ptr<Meta> sp;
        try{
            sp = category.as<shared_ptr<Meta>>();
        }catch(...){
            continue;
        } // not a category
        //auto group = m_pWindow->findChild<QFormLayout*>(category.key.c_str());
        try{
            m_WidgetMap.at(category.key);
        }catch(...){
            continue;
        }
        map<string,QWidget*>& option_map = m_WidgetMap.at(category.key);
        //LOGf("category.key: %s", category.key);
        for(auto&& op: *sp)
        {
            shared_ptr<Meta> m;
            try{
                m = op.as<shared_ptr<Meta>>();
            }catch(...){
                continue;
            }
            //LOG(string() + "\t" + op.key);
            
            QWidget* w;
            try{
                w = option_map.at(op.key);
            }catch(...){
                continue;
            }
            //auto w = label->buddy();
            bool done = false;
            auto slider = qobject_cast<QSlider*>(w);
            if(slider){
                LOG("slider");
                LOGf("%s", slider->value());
                done = true;
            }
            if(not done){
                auto combobox = qobject_cast<QComboBox*>(w);
                if(combobox)
                {
                    LOG("combobox");
                    //LOGf("%s", combobox->currentIndex());
                    done = true;
                }
            }
            if(not done){
                auto le = qobject_cast<QLineEdit*>(w);
                if(le){
                    LOG("lineedit");
                    le->setText(as_string(m, ".default").c_str());
                    //LOGf("%s", le->text().toStdString());
                    done = true;
                }
            }
            if(not done){
                auto cb = qobject_cast<QCheckBox*>(w);
                if(cb){
                    LOG("checkbox");
                    cb->setChecked(m->at<bool>(".default",false));
                    LOGf("%s", cb->checkState());
                    done = true;
                }
            }

            //rows->dumpObjectTree();
            //LOGf("%s", le);
            //LOG("---");
            //auto rows = group->findChild<QVBoxLayout*>(op);
            //cout << rows.count() << endl;
            //for(int i=0;i<rows->rowCount();++i)
            //for(int i=0;i<rows.count();++i)
            //{
                //LOG("---")
                //rows.at(i)->dumpObjectTree();
                ////auto le = rows.at(i)->findChild<QLineEdit*>();
                ////auto e = text.at(i);
                ////auto typs = le->property("type").toString().toStdString();
                ////auto typs = rows->getItem(i)->property("type").toString().toStdString();
                //cout << i << endl;
                
                //if(typs=="string")
                //{
                //    //LOG(as_string(m,".default"));
                //    //text.at(i)->setText(as_string(m,".default").c_str());
                //}
            //}
            
            //try{
            //    sp->set(op.key, m->at<string>(".default"));
            //}catch(...){
            //    try{
            //        sp->set(op.key, m->at<int>(".default"));
            //    }catch(...){
            //        try{
            //            sp->set(op.key, m->at<double>(".default"));
            //        }catch(...){
            //            try{
            //                sp->set(op.key, m->at<bool>(".default"));
            //            }catch(...){
            //                // ???
            //            }
            //        }
            //    }
            //}
        }
    }
}

void FigApp :: save_and_quit()
{
    // TODO: save
    LOG("save_and_quit");
    save();
    QApplication::quit();
}
    
void FigApp :: quit()
{
    QApplication::quit();
}

