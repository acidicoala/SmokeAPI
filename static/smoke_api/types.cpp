#include "smoke_api/types.hpp"

std::vector<DLC> DLC::get_dlcs_from_apps(const AppDlcNameMap& apps, const AppId_t app_id) {
    std::vector<DLC> dlcs;

    if(const auto app_id_str = std::to_string(app_id); apps.contains(app_id_str)) {
        const auto& app = apps.at(app_id_str);

        for(auto const& [id, name] : app.dlcs) {
            dlcs.emplace_back(id, name);
        }
    }

    return dlcs;
}

DlcNameMap DLC::get_dlc_map_from_vector(const std::vector<DLC>& dlcs) {
    DlcNameMap map;

    for(const auto& dlc : dlcs) {
        map[dlc.get_id_str()] = dlc.get_name();
    }

    return map;
}
