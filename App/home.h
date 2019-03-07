#ifndef HOME_H
#define HOME_H

#include <QObject>
#include "defaults.h"
#include "tab.h"

#include "IQ_api/client.hpp"
#include "main.h"

#include <unistd.h>
#include <string>
#include <map>
#include <vector>

class Home: public QObject
{
    Q_OBJECT
  public:
    explicit Home( std::map<std::string,std::shared_ptr<Tab>> tab_map);

  private:
    std::map<std::string,std::shared_ptr<Tab>> tab_map_;
    std::string clients_folder_path_ = ":/IQ_api/clients/";
    std::map<std::string, Client*> sys_map_;

  signals:

  public slots:
    void SetDefaults(Json::Value defaults);

};

#endif // HOME_H
