#pragma once

#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "components/brightness/BrightnessController.h"
#include "systemtask/SystemTask.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class QrViewer : public Screen {
      public:
        QrViewer(DisplayApp* app,
                 Pinetime::Controllers::DateTime& dtc,
                 Pinetime::Controllers::BrightnessController &bc,
                 System::SystemTask& st);
        ~QrViewer() override;
        void Refresh() override;
      private:

        using BrightnessLevels = Pinetime::Controllers::BrightnessController::Levels;
        
        lv_obj_t *qr;
        Pinetime::Controllers::DateTime& dateTimeController;
        Pinetime::Controllers::BrightnessController& brightnessController;
        System::SystemTask& systemTask;

        lv_point_t timeoutLinePoints[2] {{0, 1}, {239, 1}};
        lv_obj_t* timeoutLine = nullptr;
        lv_obj_t* container = nullptr;
        TickType_t timeoutTickCountStart;

        TickType_t timeoutLength;

        lv_task_t* taskRefresh;

        BrightnessLevels brightnessLevel;

        void qrCodeGen();
        void qrCodeDel();
      };
    }
  }
}

