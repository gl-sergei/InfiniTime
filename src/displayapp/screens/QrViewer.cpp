#include "displayapp/screens/QrViewer.h"
#include "displayapp/DisplayApp.h"
#include <lv_lib_qrcode/lv_qrcode.h>

#include <totp/totp.h>
#include "QrViewer_constants.h"

using namespace Pinetime::Applications::Screens;
using namespace Pinetime::Controllers;

QrViewer::QrViewer(DisplayApp* app, DateTime& dtc, BrightnessController &bc, System::SystemTask& st)
: Screen(app), dateTimeController(dtc), brightnessController(bc), systemTask(st) {
  qrCodeGen();
  brightnessLevel = brightnessController.Level();
  brightnessController.Set(BrightnessLevels::High);
  systemTask.PushMessage(System::Messages::DisableSleeping);
  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

void QrViewer::qrCodeGen() {
  auto now = std::chrono::time_point_cast<std::chrono::seconds>(dateTimeController.UTCDateTime()).time_since_epoch().count();
  unsigned long ltm = now / totpIntervalSec;
  int secondsPassed = now % totpIntervalSec;
  timeoutTickCountStart = xTaskGetTickCount() - pdMS_TO_TICKS(secondsPassed * 1000);
  timeoutLength = pdMS_TO_TICKS(totpIntervalSec * 1000);

  /* progress bar */
  timeoutLine = lv_line_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_line_width(timeoutLine, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_line_color(timeoutLine, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
  lv_obj_set_style_local_line_rounded(timeoutLine, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, true);

  lv_line_set_points(timeoutLine, timeoutLinePoints, 2);

  /* container */
  container = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_style_local_border_color(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_style_local_pad_top(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_pad_inner(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_border_width(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 8);
  lv_obj_set_auto_realign(container, true);
  lv_obj_align_origo(container, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_cont_set_fit(container, LV_FIT_TIGHT);
  lv_cont_set_layout(container, LV_LAYOUT_COLUMN_MID);

  /* QR code */
  int code = generateCode(totpSecret, totpSecretLen, ltm, _6digits);
  qr = lv_qrcode_create(container, qrCodeWidth, LV_COLOR_BLACK, LV_COLOR_WHITE);

  /*Set data*/
  char data[qrTextLen + 1];
  sprintf(data, qrTextTemplate, code);
  lv_qrcode_update(qr, data);

  lv_obj_set_pos(qr, 0, 0);
}

void QrViewer::qrCodeDel() {
  lv_obj_del(timeoutLine);
  timeoutLine = nullptr;
  lv_qrcode_delete(qr);
  lv_obj_del(container);
  lv_obj_clean(lv_scr_act());
}

void QrViewer::Refresh() {
  TickType_t tick = xTaskGetTickCount();
  if (tick - timeoutTickCountStart > pdMS_TO_TICKS(totpIntervalSec * 1000)) {
    /* totp timer expired, generate new qr code */
    qrCodeDel();
    qrCodeGen();
  }
  int32_t pos = LV_HOR_RES - ((tick - timeoutTickCountStart) / (timeoutLength / LV_HOR_RES));
  timeoutLinePoints[1].x = pos;
  lv_line_set_points(timeoutLine, timeoutLinePoints, 2);
}

QrViewer::~QrViewer() {
  lv_task_del(taskRefresh);
  brightnessController.Set(brightnessLevel);
  systemTask.PushMessage(System::Messages::EnableSleeping);
  qrCodeDel();
}
