#include "jetpwmon/jetpwmon++.hpp"
#include "jetpwmon/src/lib.rs.h"

std::unique_ptr<jetpwmon::PowerMonitor> new_power_monitor() {
    return std::make_unique<jetpwmon::PowerMonitor>();
} 