#pragma once

#include <etl/variant.h>

#include "protos/ThermoCommEvent_bp.h"

namespace CommEvent {

typedef etl::variant<TempChangedEvent, SetPointChangedEvent,
                     HeatingModeChangedEvent, SettingsChangedEvent>
    CommEventType;

void PrintCommEventToSerial(CommEventType& ev);

}  // namespace CommEvent
