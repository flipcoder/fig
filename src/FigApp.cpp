#include "FigApp.h"
#include "FigWindow.h"
#include <vector>
#include <fstream>
#include "kit/kit.h"
#include "kit/log/log.h"
#include <QDialogButtonBox>
#include <boost/filesystem.hpp>
using namespace std;
using kit::make_unique;
namespace fs = boost::filesystem;
using path = boost::filesystem::path;

FigApp :: FigApp(int& argc, char* argv[]):
    QApplication(argc,argv),
    m_Args(argc, (const char**)argv),
    m_FigAppPath(argv[0])
{
    path appPath = path(m_FigAppPath).parent_path();
    
    if(m_Args.size()>0)
        m_SchemaFn = m_Args.get(m_Args.size()-1);
    else
        m_SchemaFn = (appPath / m_SchemaFn).string();

    path schemaPath = path(m_SchemaFn).parent_path();
    
    try{
        m_pSchema = make_shared<Meta>(m_SchemaFn);
    }catch(Error& e){
        QMessageBox::critical(m_pWindow.get(), "Error", "Could not load schema file.");
        fail();
        goto return_ctor;
    }

    // use .settings path or default to same path as schema
    if(m_pSchema->has(".settings"))
        m_SettingsFn = m_pSchema->at<string>(".settings");
    else
        m_SettingsFn = (schemaPath / m_SettingsFn).string();
    
    if(fs::exists(m_SettingsFn))
    {
        try{
            m_pSettings = make_shared<Meta>(m_SettingsFn);
        }catch(...){
            QMessageBox::critical(m_pWindow.get(), "Error",
                "Unable to load settings.  The file may be corrupt."
            );
            m_pSettings = make_shared<Meta>();
        }
    }
    else
        m_pSettings = make_shared<Meta>();
        
    m_Title = m_Args.value_or("title", "Settings");
    if(m_pSchema->has(".title"))
        m_Title = m_pSchema->at<string>(".title");
 
    if(!init()){
        QMessageBox::critical(m_pWindow.get(), "Error", "Failed to initialize schema.");
        fail();
        goto return_ctor;
    }
        
    m_ReturnCode = RC_SUCCESS; // ctor passed, tell main() we didn't fail

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

bool FigApp :: init()
{
    m_pWindow = make_unique<FigWindow>(this);
    
    QVBoxLayout* layout = new QVBoxLayout;
    //layout->setObjectName("layout");

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
            }catch(...){
                try {
                    typid = op->meta(".values")->type_id(0);
                }catch(...){}
            }
            bool bools = false;
            string typ;
            if(typid==MetaType::ID::STRING)
                typ="string";
            else if(typid==MetaType::ID::INT)
                typ="int";
            else if(typid==MetaType::ID::REAL)
                typ="double";
            else if(typid==MetaType::ID::BOOL){
                typ="bool";
                bools=true;
            }

            // check if values are bools
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
                //box->setObjectName("box");
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

    if(!load())
        return false;
    
    m_pWindow->show();

    return true;
}

bool FigApp :: save()
{
    bool fail = false;
    for(auto&& category: *m_pSchema)
    {
        shared_ptr<Meta> sp;
        try{
            sp = category.as<shared_ptr<Meta>>();
        }catch(...){
            continue;
        } // not a category
        try{
            m_WidgetMap.at(category.key);
        }catch(...){
            continue;
        }
        map<string,QWidget*>& option_map = m_WidgetMap.at(category.key);
        shared_ptr<Meta> setting;
        try{
            setting = m_pSettings->meta(category.key);
        }catch(...){
        }
        if(not setting){
            m_pSettings->set<shared_ptr<Meta>>(category.key, make_shared<Meta>()); // create
            setting = m_pSettings->meta(category.key);
        }
        for(auto&& op: *sp)
        {
            shared_ptr<Meta> m;
            try{
                m = op.as<shared_ptr<Meta>>();
            }catch(...){
                continue;
            }
            
            QWidget* w;
            try{
                w = option_map.at(op.key);
            }catch(...){
                continue;
            }
            bool cast = false;
            auto slider = qobject_cast<QSlider*>(w);
            if(slider){
                //int v = m->at<int>(".default",0);
                setting->set<int>(op.key, slider->value());
                cast = true;
            }
            if(not cast){
                auto combobox = qobject_cast<QComboBox*>(w);
                if(combobox)
                {
                    auto typ = combobox->property("type");
                    auto idx = combobox->currentIndex();
                    if(typ=="string"){
                        try{
                            setting->set<string>(op.key, m->meta(".values")->at<string>(idx));
                        }catch(boost::bad_any_cast&){}
                    }else if(typ=="int"){
                        try{
                            setting->set<int>(op.key, m->meta(".values")->at<int>(idx));
                        }catch(boost::bad_any_cast&){}
                    }else if(typ=="bool"){
                        try{
                            setting->set<bool>(op.key, m->meta(".values")->at<bool>(idx));
                        }catch(boost::bad_any_cast&){}
                    }else if(typ=="double"){
                        try{
                            setting->set<double>(op.key, m->meta(".values")->at<double>(idx));
                        }catch(boost::bad_any_cast&){}
                    }


                    cast = true;
                }
            }
            if(not cast){
                auto le = qobject_cast<QLineEdit*>(w);
                if(le){
                    auto typ = le->property("type");
                    if(typ=="string")
                        setting->set<string>(op.key, le->text().toStdString());
                    else if(typ=="int"){
                        try{
                            setting->set<int>(op.key, stoi(le->text().toStdString()));
                        }catch(...){
                            QMessageBox::critical(m_pWindow.get(), "Error",
                                (m->at<string>(".name", op.key) + " needs to be an integer.").c_str()
                            );
                            fail = true;
                            break;
                        }
                    }else if(typ=="double"){
                        try{
                            setting->set<double>(op.key, stod(le->text().toStdString()));
                        }catch(...){
                            QMessageBox::critical(m_pWindow.get(), "Error",
                                (m->at<string>(".name", op.key) + " needs to be a number.").c_str()
                            );
                            fail = true;
                            break;
                        }
                    }

                    
                    cast = true;
                }
            }
            if(not cast){
                auto cb = qobject_cast<QCheckBox*>(w);
                if(cb){
                    setting->set<bool>(op.key, cb->checkState());
                    cast = true;
                }
            }
        }
    }
    if(not fail)
    {
        try{
            m_pSettings->serialize(m_SettingsFn);
        }catch(...){
            QMessageBox::critical(m_pWindow.get(), "Error",
                "Unable to save settings."
            );
            fail = true;
        }
    }
    return not fail;
}

bool FigApp :: load()
{
    for(auto&& category: *m_pSettings)
    {
        shared_ptr<Meta> sp;
        try{
            sp = category.as<shared_ptr<Meta>>();
        }catch(...){
            continue;
        } // not a category
        try{
            m_WidgetMap.at(category.key);
        }catch(...){
            continue;
        }
        map<string,QWidget*>& option_map = m_WidgetMap.at(category.key);
        for(auto&& op: *sp)
        {
            shared_ptr<Meta> m;
            try{
                m = m_pSchema->meta(category.key)->meta(op.key);
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
            bool cast = false;
            auto slider = qobject_cast<QSlider*>(w);
            if(slider){
                slider->setValue(op.as<int>());
                cast = true;
            }
            if(not cast){
                auto combobox = qobject_cast<QComboBox*>(w);
                if(combobox)
                {
                    int idx = 0;
                    try{
                        auto s = op.as<string>();
                        idx = index_of_meta(m, s);
                    }catch(...){
                    }
                    try{
                        auto b = op.as<bool>();
                        idx = index_of_meta(m, b);
                    }catch(...){
                    }
                    try{
                        int v = op.as<int>();
                        idx = index_of_meta(m, v);
                    }catch(...){
                    }
                    try{
                        double v = op.as<double>();
                        idx = index_of_meta(m, v);
                    }catch(...){
                    }
                    combobox->setCurrentIndex(idx);
                    cast = true;
                }
            }
            if(not cast){
                auto le = qobject_cast<QLineEdit*>(w);
                if(le){
                    le->setText(as_string(op).c_str());
                    cast = true;
                }
            }
            if(not cast){
                auto cb = qobject_cast<QCheckBox*>(w);
                if(cb){
                    cb->setChecked(op.as<bool>());
                    cast = true;
                }
            }
        }
    }
    
    return true;
}

template<class T>
int FigApp :: index_of_meta(shared_ptr<Meta>& m, T v)
{
    int i = 0;
    for(auto&& e: *m)
    {
        if(e.as<T>() == v)
            return i;
        ++i;
    }
    throw std::out_of_range("");
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
            bool cast = false;
            auto slider = qobject_cast<QSlider*>(w);
            if(slider){
                int v = m->at<int>(".default",0);
                slider->setValue(v);
                cast = true;
            }
            if(not cast){
                auto combobox = qobject_cast<QComboBox*>(w);
                if(combobox)
                {
                    int defidx = 0;
                    try{
                        auto s = m->at<string>(".default");
                        defidx = index_of_meta(m, s);
                    }catch(...){
                    }
                    try{
                        auto b = m->at<bool>(".default");
                        defidx = index_of_meta(m, b);
                    }catch(...){
                    }
                    try{
                        int v = m->at<int>(".default");
                        defidx = index_of_meta(m, v);
                    }catch(...){
                    }
                    try{
                        auto v = m->at<double>(".default");
                        defidx = index_of_meta(m, v);
                    }catch(...){
                    }
                    combobox->setCurrentIndex(defidx);
                    cast = true;
                }
            }
            if(not cast){
                auto le = qobject_cast<QLineEdit*>(w);
                if(le){
                    le->setText(as_string(m, ".default").c_str());
                    cast = true;
                }
            }
            if(not cast){
                auto cb = qobject_cast<QCheckBox*>(w);
                if(cb){
                    cb->setChecked(m->at<bool>(".default",false));
                    cast = true;
                }
            }
        }
    }
}

void FigApp :: save_and_quit()
{
    if(save()){
        m_ReturnCode = RC_SUCCESS;
        if(m_pSchema->has(".launch")){
            auto launch = m_pSchema->at<string>(".launch");
            if(!QProcess::startDetached(launch.c_str())){
                string e = string("Unable to launch \"") + launch + "\".";
                QMessageBox::critical(m_pWindow.get(), "Error", e.c_str());
                m_ReturnCode = RC_ERROR;
            }
        }
        QApplication::quit();
    }
    else{
        // failed save() calls show dialog
        m_ReturnCode = RC_ERROR;
        QApplication::quit();
    }
}
    
void FigApp :: quit()
{
    m_ReturnCode = RC_CLOSE;
    QApplication::quit();
}

void FigApp :: fail()
{
    m_ReturnCode = RC_ERROR;
    QApplication::quit();
}

