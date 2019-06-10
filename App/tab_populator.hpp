#ifndef TAB_POPULATOR_HPP
#define TAB_POPULATOR_HPP

#include <QObject>
#include "tab.h"
#include "IQ_api/json_cpp.hpp"

#include <map>

class TabPopulator : public QObject
{
    Q_OBJECT
  private:
    Ui::MainWindow *ui_;
    std::map<std::string,std::shared_ptr<Tab>> *tab_map_;
    JsonCpp json_;
    Json::Value firmware_styles_;
    uint32_t firmware_index_;
    std::string firmware_name_;

    void LoadFirmwareStylesFromHardwareType(const int &hardware_type);
    Json::Value OpenAndLoadJsonFile(const QString &file_path);
    Json::Value LoadJsonFile(QFile &my_file);

    void FindFirmwareIndex(const int &firmware_style);

    void GetAndDisplayFirmwareName();
    void GetFirmwareName();
    void DisplayFirmwareName();

    void CheckMinFirmwareBuildNumber(const int &firmware_build_number);
    void DisplayUpdateFirmwareWarning();

    void CreateTabFrames();
    std::map<QWidget*,std::vector<std::string>> LinkTabWidgetAndFirmwareFiles();
    void UpdateTabMap(std::shared_ptr<Tab> &tab, std::string &tab_name);


  public:
    TabPopulator(Ui::MainWindow *ui, std::map<std::string,std::shared_ptr<Tab>> *tab_map);

  signals:

  public slots:
    void PopulateTabs(int hardware_type, int firmware_style, int firmware_build_number);
};

#endif // TAB_POPULATOR_HPP
