#include "displayapp/screens/List.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void ButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<List*>(obj->user_data);
    screen->OnButtonEvent(obj, event);
  }
}

List::List(uint8_t screenID,
           uint8_t numScreens,
           DisplayApp* app,
           Controllers::Settings& settingsController,
           std::array<Applications, MAXLISTITEMS>& applications)
  : Screen(app), settingsController {settingsController}, pageIndicator(screenID, numScreens) {

  // Set the background to Black
  lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0, 0, 0));

  settingsController.SetSettingsMenu(screenID);

  pageIndicator.Create();

  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container1, 0, 0);
  lv_obj_set_width(container1, LV_HOR_RES - 8);
  lv_obj_set_height(container1, LV_VER_RES);
  lv_cont_set_layout(container1, LV_LAYOUT_COLUMN_LEFT);

  lv_obj_t* labelBt;
  lv_obj_t* labelBtIco;

  for (int i = 0; i < MAXLISTITEMS; i++) {
    apps[i] = applications[i].application;
    if (applications[i].application != Apps::None) {

      itemApps[i] = lv_btn_create(container1, nullptr);
      lv_obj_set_style_local_bg_opa(itemApps[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
      lv_obj_set_style_local_radius(itemApps[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 57);
      lv_obj_set_style_local_bg_color(itemApps[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);

      lv_obj_set_width(itemApps[i], LV_HOR_RES - 8);
      lv_obj_set_height(itemApps[i], 57);
      lv_obj_set_event_cb(itemApps[i], ButtonEventHandler);
      lv_btn_set_layout(itemApps[i], LV_LAYOUT_ROW_MID);
      itemApps[i]->user_data = this;

      labelBtIco = lv_label_create(itemApps[i], nullptr);
      lv_obj_set_style_local_text_color(labelBtIco, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
      lv_label_set_text_static(labelBtIco, applications[i].icon);

      labelBt = lv_label_create(itemApps[i], nullptr);
      lv_label_set_text_fmt(labelBt, " %s", applications[i].name);
    }
  }
}

List::~List() {
  lv_obj_clean(lv_scr_act());
}

void List::OnButtonEvent(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    for (int i = 0; i < MAXLISTITEMS; i++) {
      if (apps[i] != Apps::None && object == itemApps[i]) {
        app->StartApp(apps[i], DisplayApp::FullRefreshDirections::Up);
        running = false;
        return;
      }
    }
  }
}
