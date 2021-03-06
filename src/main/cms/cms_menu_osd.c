/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "platform.h"

#if defined(OSD) && defined(CMS)

#include "build/version.h"

#include "cms/cms.h"
#include "cms/cms_types.h"
#include "cms/cms_menu_osd.h"

#include "common/utils.h"

#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "io/displayport_max7456.h"
#include "io/osd.h"

static uint8_t osdConfig_rssi_alarm;
static uint16_t osdConfig_cap_alarm;
static uint16_t osdConfig_time_alarm;
static uint16_t osdConfig_alt_alarm;

static long cmsx_menuAlarmsOnEnter(void)
{
    osdConfig_rssi_alarm = osdConfig()->rssi_alarm;
    osdConfig_cap_alarm = osdConfig()->cap_alarm;
    osdConfig_time_alarm = osdConfig()->time_alarm;
    osdConfig_alt_alarm = osdConfig()->alt_alarm;
    return 0;
}

static long cmsx_menuAlarmsOnExit(const OSD_Entry *self)
{
    UNUSED(self);

    osdConfigMutable()->rssi_alarm = osdConfig_rssi_alarm;
    osdConfigMutable()->cap_alarm = osdConfig_cap_alarm;
    osdConfigMutable()->time_alarm = osdConfig_time_alarm;
    osdConfigMutable()->alt_alarm = osdConfig_alt_alarm;
    return 0;
}

OSD_Entry cmsx_menuAlarmsEntries[] =
{
    {"--- ALARMS ---", OME_Label, NULL, NULL, 0},
    {"RSSI",     OME_UINT8,  NULL, &(OSD_UINT8_t){&osdConfig_rssi_alarm, 5, 90, 5}, 0},
    {"MAIN BAT", OME_UINT16, NULL, &(OSD_UINT16_t){&osdConfig_cap_alarm, 50, 30000, 50}, 0},
    {"FLY TIME", OME_UINT16, NULL, &(OSD_UINT16_t){&osdConfig_time_alarm, 1, 200, 1}, 0},
    {"MAX ALT",  OME_UINT16, NULL, &(OSD_UINT16_t){&osdConfig_alt_alarm, 1, 200, 1}, 0},
    {"BACK", OME_Back, NULL, NULL, 0},
    {NULL, OME_END, NULL, NULL, 0}
};

CMS_Menu cmsx_menuAlarms = {
    .GUARD_text = "MENUALARMS",
    .GUARD_type = OME_MENU,
    .onEnter = cmsx_menuAlarmsOnEnter,
    .onExit = cmsx_menuAlarmsOnExit,
    .onGlobalExit = NULL,
    .entries = cmsx_menuAlarmsEntries,
};

#ifndef DISABLE_EXTENDED_CMS_OSD_MENU
static uint16_t osdConfig_item_pos[OSD_ITEM_COUNT];

static long menuOsdActiveElemsOnEnter(void)
{
    memcpy(&osdConfig_item_pos[0], &osdConfig()->item_pos[0], sizeof(uint16_t) * OSD_ITEM_COUNT);
    return 0;
}

static long menuOsdActiveElemsOnExit(const OSD_Entry *self)
{
    UNUSED(self);

    memcpy(&osdConfigMutable()->item_pos[0], &osdConfig_item_pos[0], sizeof(uint16_t) * OSD_ITEM_COUNT);
    return 0;
}

OSD_Entry menuOsdActiveElemsEntries[] =
{
    {"--- ACTIV ELEM ---", OME_Label,   NULL, NULL, 0},
    {"RSSI",               OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_RSSI_VALUE], 0},
    {"BATTERY VOLTAGE",    OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_MAIN_BATT_VOLTAGE], 0},
    {"BATTERY USAGE",      OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_MAIN_BATT_USAGE], 0},
    {"AVG CELL VOLTAGE",   OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_AVG_CELL_VOLTAGE], 0},
    {"CROSSHAIRS",         OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_CROSSHAIRS], 0},
    {"HORIZON",            OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_ARTIFICIAL_HORIZON], 0},
    {"HORIZON SIDEBARS",   OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_HORIZON_SIDEBARS], 0},
    {"UPTIME",             OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_ONTIME], 0},
    {"FLY TIME",           OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_FLYTIME], 0},
    {"FLY MODE",           OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_FLYMODE], 0},
    {"NAME",               OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_CRAFT_NAME], 0},
    {"THROTTLE",           OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_THROTTLE_POS], 0},
#ifdef VTX_CONTROL
    {"VTX CHAN",           OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_VTX_CHANNEL], 0},
#endif // VTX
    {"CURRENT (A)",        OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_CURRENT_DRAW], 0},
    {"USED MAH",           OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_MAH_DRAWN], 0},
#ifdef GPS
    {"GPS SPEED",          OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_GPS_SPEED], 0},
    {"GPS SATS",           OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_GPS_SATS], 0},
    {"GPS LAT",            OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_GPS_LAT], 0},
    {"GPS LON",            OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_GPS_LON], 0},
    {"HOME DIR",           OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_HOME_DIR], 0},
    {"HOME DIST",          OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_HOME_DIST], 0},
#endif // GPS
    {"COMPASS BAR",        OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_COMPASS_BAR], 0},
    {"ALTITUDE",           OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_ALTITUDE], 0},
    {"POWER",              OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_POWER], 0},
    {"ROLL PID",           OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_ROLL_PIDS], 0},
    {"PITCH PID",          OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_PITCH_PIDS], 0},
    {"YAW PID",            OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_YAW_PIDS], 0},
    {"PROFILES",           OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_PIDRATE_PROFILE], 0},
    {"DEBUG",              OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_DEBUG], 0},
    {"BATT WARN",          OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_MAIN_BATT_WARNING], 0},
    {"DISARMED",           OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_DISARMED], 0},
    {"PIT ANG",            OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_PITCH_ANGLE], 0},
    {"ROL ANG",            OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_ROLL_ANGLE], 0},
    {"ARMED TIME",         OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_ARMED_TIME], 0},
    {"HEADING",            OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_NUMERICAL_HEADING], 0},
    {"VARIO",              OME_VISIBLE, NULL, &osdConfig_item_pos[OSD_NUMERICAL_VARIO], 0},
    {"BACK",               OME_Back,    NULL, NULL, 0},
    {NULL,                 OME_END,     NULL, NULL, 0}
};

CMS_Menu menuOsdActiveElems = {
    .GUARD_text = "MENUOSDACT",
    .GUARD_type = OME_MENU,
    .onEnter = menuOsdActiveElemsOnEnter,
    .onExit = menuOsdActiveElemsOnExit,
    .onGlobalExit = NULL,
    .entries = menuOsdActiveElemsEntries
};
#endif /* DISABLE_EXTENDED_CMS_OSD_MENU */

#ifdef USE_MAX7456
static bool displayPortProfileMax7456_invert;
static uint8_t displayPortProfileMax7456_blackBrightness;
static uint8_t displayPortProfileMax7456_whiteBrightness;
#endif

static long cmsx_menuOsdOnEnter(void)
{
#ifdef USE_MAX7456
    displayPortProfileMax7456_invert = displayPortProfileMax7456()->invert;
    displayPortProfileMax7456_blackBrightness = displayPortProfileMax7456()->blackBrightness;
    displayPortProfileMax7456_whiteBrightness = displayPortProfileMax7456()->whiteBrightness;
#endif

    return 0;
}

static long cmsx_menuOsdOnExit(const OSD_Entry *self)
{
    UNUSED(self);

#ifdef USE_MAX7456
    displayPortProfileMax7456Mutable()->invert = displayPortProfileMax7456_invert;
    displayPortProfileMax7456Mutable()->blackBrightness = displayPortProfileMax7456_blackBrightness;
    displayPortProfileMax7456Mutable()->whiteBrightness = displayPortProfileMax7456_whiteBrightness;
#endif

  return 0;
}

OSD_Entry cmsx_menuOsdEntries[] =
{
    {"---OSD---",   OME_Label,   NULL,          NULL,                0},
#ifndef DISABLE_EXTENDED_CMS_OSD_MENU
    {"ACTIVE ELEM", OME_Submenu, cmsMenuChange, &menuOsdActiveElems, 0},
#endif
#ifdef USE_MAX7456
    {"INVERT",    OME_Bool,  NULL, &displayPortProfileMax7456_invert,                                   0},
    {"BRT BLACK", OME_UINT8, NULL, &(OSD_UINT8_t){&displayPortProfileMax7456_blackBrightness, 0, 3, 1}, 0},
    {"BRT WHITE", OME_UINT8, NULL, &(OSD_UINT8_t){&displayPortProfileMax7456_whiteBrightness, 0, 3, 1}, 0},
#endif
    {"BACK", OME_Back, NULL, NULL, 0},
    {NULL,   OME_END,  NULL, NULL, 0}
};

CMS_Menu cmsx_menuOsd = {
    .GUARD_text = "MENUOSD",
    .GUARD_type = OME_MENU,
    .onEnter = cmsx_menuOsdOnEnter,
    .onExit = cmsx_menuOsdOnExit,
    .onGlobalExit = NULL,
    .entries = cmsx_menuOsdEntries
};
#endif // CMS
