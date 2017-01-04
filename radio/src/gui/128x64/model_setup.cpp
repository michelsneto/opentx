/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x 
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "opentx.h"

#if defined(CPUARM)
uint8_t g_moduleIdx;
void menuModelFailsafe(event_t event);
#endif

enum MenuModelSetupItems {
  ITEM_MODEL_NAME,
  ITEM_MODEL_TIMER1,
  CASE_CPUARM(ITEM_MODEL_TIMER1_NAME)
  CASE_PERSISTENT_TIMERS(ITEM_MODEL_TIMER1_PERSISTENT)
  ITEM_MODEL_TIMER1_MINUTE_BEEP,
  ITEM_MODEL_TIMER1_COUNTDOWN_BEEP,
  ITEM_MODEL_TIMER2,
  CASE_CPUARM(ITEM_MODEL_TIMER2_NAME)
  CASE_PERSISTENT_TIMERS(ITEM_MODEL_TIMER2_PERSISTENT)
  ITEM_MODEL_TIMER2_MINUTE_BEEP,
  ITEM_MODEL_TIMER2_COUNTDOWN_BEEP,
  CASE_CPUARM(ITEM_MODEL_TIMER3)
  CASE_CPUARM(ITEM_MODEL_TIMER3_NAME)
  CASE_CPUARM(ITEM_MODEL_TIMER3_PERSISTENT)
  CASE_CPUARM(ITEM_MODEL_TIMER3_MINUTE_BEEP)
  CASE_CPUARM(ITEM_MODEL_TIMER3_COUNTDOWN_BEEP)
  ITEM_MODEL_EXTENDED_LIMITS,
  ITEM_MODEL_EXTENDED_TRIMS,
  CASE_CPUARM(ITEM_MODEL_DISPLAY_TRIMS)
  ITEM_MODEL_TRIM_INC,
  ITEM_MODEL_THROTTLE_REVERSED,
  ITEM_MODEL_THROTTLE_TRACE,
  ITEM_MODEL_THROTTLE_TRIM,
  CASE_CPUARM(ITEM_MODEL_PREFLIGHT_LABEL)
  CASE_CPUARM(ITEM_MODEL_CHECKLIST_DISPLAY)
  ITEM_MODEL_THROTTLE_WARNING,
  ITEM_MODEL_SWITCHES_WARNING,
  ITEM_MODEL_BEEP_CENTER,
  CASE_CPUARM(ITEM_MODEL_USE_GLOBAL_FUNCTIONS)
#if defined(PCBX7)
  ITEM_MODEL_INTERNAL_MODULE_LABEL,
  ITEM_MODEL_INTERNAL_MODULE_MODE,
  ITEM_MODEL_INTERNAL_MODULE_CHANNELS,
  ITEM_MODEL_INTERNAL_MODULE_BIND,
  ITEM_MODEL_INTERNAL_MODULE_FAILSAFE,
#endif
#if defined(CPUARM)
  ITEM_MODEL_EXTERNAL_MODULE_LABEL,
  ITEM_MODEL_EXTERNAL_MODULE_MODE,
#if defined(MULTIMODULE)
  ITEM_MODEL_EXTERNAL_MODULE_SUBTYPE,
  ITEM_MODEL_EXTERNAL_MODULE_STATUS,
#endif
  ITEM_MODEL_EXTERNAL_MODULE_CHANNELS,
  ITEM_MODEL_EXTERNAL_MODULE_BIND,
#if defined(PCBSKY9X) && defined(REVX)
  ITEM_MODEL_EXTERNAL_MODULE_OUTPUT_TYPE,
#endif
  ITEM_MODEL_EXTERNAL_MODULE_FAILSAFE,
#if defined(MULTIMODULE)
  ITEM_MODEL_EXTERNAL_MODULE_AUTOBIND,
  ITEM_MODEL_EXTERNAL_MODULE_LOWPOWER,
#endif
#if defined(PCBSKY9X) && !defined(REVA)
  ITEM_MODEL_EXTRA_MODULE_LABEL,
  ITEM_MODEL_EXTRA_MODULE_CHANNELS,
  ITEM_MODEL_EXTRA_MODULE_BIND,
#endif
#else
  ITEM_MODEL_PPM1_PROTOCOL,
  ITEM_MODEL_PPM1_PARAMS,
#endif
#if defined(PCBX7)
  ITEM_MODEL_TRAINER_LABEL,
  ITEM_MODEL_TRAINER_MODE,
  ITEM_MODEL_TRAINER_CHANNELS,
  ITEM_MODEL_TRAINER_SETTINGS,
#endif
  ITEM_MODEL_SETUP_MAX
};

#if defined(PCBSKY9X)
  #define FIELD_PROTOCOL_MAX             2
#else
  #define FIELD_PROTOCOL_MAX             1
#endif

#define MODEL_SETUP_2ND_COLUMN           (LCD_W-11*FW)
#define MODEL_SETUP_BIND_OFS             2*FW+1
#define MODEL_SETUP_RANGE_OFS            4*FW+3
#define MODEL_SETUP_SET_FAILSAFE_OFS     7*FW-2

#if defined(PCBX7)
  #define CURRENT_MODULE_EDITED(k)       (k>=ITEM_MODEL_TRAINER_LABEL ? TRAINER_MODULE : (k>=ITEM_MODEL_EXTERNAL_MODULE_LABEL ? EXTERNAL_MODULE : INTERNAL_MODULE))
#elif defined(PCBSKY9X) && !defined(REVA)
  #define CURRENT_MODULE_EDITED(k)       (k>=ITEM_MODEL_EXTRA_MODULE_LABEL ? EXTRA_MODULE : EXTERNAL_MODULE)
#else
  #define CURRENT_MODULE_EDITED(k)       (EXTERNAL_MODULE)
#endif

#if defined(CPUARM)

#if !defined(TARANIS_INTERNAL_PPM)
  #define INTERNAL_MODULE_MODE_ROWS       0 // (OFF / RF protocols)
#else
  #define INTERNAL_MODULE_MODE_ROWS       (IS_MODULE_XJT(INTERNAL_MODULE) ? (uint8_t)1 : (uint8_t)0) // Module type + RF protocols
#endif
#if defined(TARANIS_INTERNAL_PPM)
  #define IF_INTERNAL_MODULE_ON(x)        (g_model.moduleData[INTERNAL_MODULE].type == MODULE_TYPE_NONE ? HIDDEN_ROW : (uint8_t)(x))
#else
  #define IF_INTERNAL_MODULE_ON(x)        (g_model.moduleData[INTERNAL_MODULE].rfProtocol == RF_PROTO_OFF ? HIDDEN_ROW : (uint8_t)(x))
#endif

  #define IF_EXTERNAL_MODULE_ON(x)       (g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_NONE ? HIDDEN_ROW : (uint8_t)(x))
  #define IF_EXTERNAL_MODULE_XJT(x)      (IS_MODULE_XJT(EXTERNAL_MODULE) ? (uint8_t)(x) : HIDDEN_ROW)
  #define INTERNAL_MODULE_CHANNELS_ROWS  IF_INTERNAL_MODULE_ON(1)
  #define EXTERNAL_MODULE_BIND_ROWS()    (IS_MODULE_XJT(EXTERNAL_MODULE) && IS_D8_RX(EXTERNAL_MODULE)) ? (uint8_t)1 : (IS_MODULE_PPM(EXTERNAL_MODULE) || IS_MODULE_XJT(EXTERNAL_MODULE) || IS_MODULE_DSM2(EXTERNAL_MODULE) || IS_MODULE_MULTIMODULE(EXTERNAL_MODULE)) ? (uint8_t)2 : HIDDEN_ROW

#if defined(PCBSKY9X) && defined(REVX)
  #define OUTPUT_TYPE_ROWS()             (IS_MODULE_PPM(EXTERNAL_MODULE) ? (uint8_t)0 : HIDDEN_ROW) ,
#else
  #define OUTPUT_TYPE_ROWS()
#endif
  #define PORT_CHANNELS_ROWS(x)          (x==EXTERNAL_MODULE ? EXTERNAL_MODULE_CHANNELS_ROWS : 0)

  #define EXTERNAL_MODULE_MODE_ROWS      (IS_MODULE_XJT(EXTERNAL_MODULE) || IS_MODULE_DSM2(EXTERNAL_MODULE) || IS_MODULE_MULTIMODULE(EXTERNAL_MODULE)) ? (uint8_t)1 : (uint8_t)0

  #define CURSOR_ON_CELL                 (true)
  #define MODEL_SETUP_MAX_LINES          (HEADER_LINE+ITEM_MODEL_SETUP_MAX)
  #define POT_WARN_ITEMS()               ((g_model.nPotsToWarn >> 6) ? (uint8_t)NUM_POTS+NUM_SLIDERS : (uint8_t)0)
  #define TIMER_ROWS                     2, 0, CASE_PERSISTENT_TIMERS(0) 0, 0
#if defined(PCBSKY9X) && !defined(REVA)
  #define EXTRA_MODULE_ROWS              LABEL(ExtraModule), 1, 2,
#else
  #define EXTRA_MODULE_ROWS
#endif

#if defined(PCBX7)
  #define TRAINER_CHANNELS_ROWS()        IF_TRAINER_ON(1)
  #define TRAINER_MODULE_ROWS            LABEL(Trainer), 0, TRAINER_CHANNELS_ROWS(), IF_TRAINER_ON(2)
#else
  #define TRAINER_MODULE_ROWS
#endif

#elif defined(CPUM64)
  #define CURSOR_ON_CELL                 (true)
  #define MODEL_SETUP_MAX_LINES          ((IS_PPM_PROTOCOL(protocol)||IS_DSM2_PROTOCOL(protocol)||IS_PXX_PROTOCOL(protocol)) ? HEADER_LINE+ITEM_MODEL_SETUP_MAX : HEADER_LINE+ITEM_MODEL_SETUP_MAX-1)
#else
  #define CURSOR_ON_CELL                 (true)
  #define MODEL_SETUP_MAX_LINES          ((IS_PPM_PROTOCOL(protocol)||IS_DSM2_PROTOCOL(protocol)||IS_PXX_PROTOCOL(protocol)) ? HEADER_LINE+ITEM_MODEL_SETUP_MAX : HEADER_LINE+ITEM_MODEL_SETUP_MAX-1)
#endif

void menuModelSetup(event_t event)
{
#if defined(PCBX7)
  MENU_TAB({ HEADER_LINE_COLUMNS 0, TIMER_ROWS, TIMER_ROWS, TIMER_ROWS, 0, 1, 0, 0, 0, 0, 0, CASE_CPUARM(LABEL(PreflightCheck)) CASE_CPUARM(0) 0, NUM_SWITCHES-1, NUM_STICKS+NUM_POTS+NUM_SLIDERS+NUM_ROTARY_ENCODERS-1, 0,
  LABEL(InternalModule),
  INTERNAL_MODULE_MODE_ROWS,
  INTERNAL_MODULE_CHANNELS_ROWS,
  IF_INTERNAL_MODULE_ON(HAS_RF_PROTOCOL_MODELINDEX(g_model.moduleData[INTERNAL_MODULE].rfProtocol) ? (uint8_t)2 : (uint8_t)1),
  IF_INTERNAL_MODULE_ON(FAILSAFE_ROWS(INTERNAL_MODULE)),
  LABEL(ExternalModule),
  EXTERNAL_MODULE_MODE_ROWS,
  MULTIMODULE_SUBTYPE_ROWS(EXTERNAL_MODULE)
  MULTIMODULE_STATUS_ROW
  EXTERNAL_MODULE_CHANNELS_ROWS,
  EXTERNAL_MODULE_BIND_ROWS(),
  OUTPUT_TYPE_ROWS()
  FAILSAFE_ROWS(EXTERNAL_MODULE),
  MULTIMODULE_MODULE_ROWS
  EXTRA_MODULE_ROWS
  TRAINER_MODULE_ROWS });
#elif defined(CPUARM)
  MENU_TAB({ HEADER_LINE_COLUMNS 0, TIMER_ROWS, TIMER_ROWS, TIMER_ROWS, 0, 1, 0, 0, 0, 0, 0, CASE_CPUARM(LABEL(PreflightCheck)) CASE_CPUARM(0) 0, NUM_SWITCHES-1, NUM_STICKS+NUM_POTS+NUM_SLIDERS+NUM_ROTARY_ENCODERS-1, 0,
  LABEL(ExternalModule),
  EXTERNAL_MODULE_MODE_ROWS,
  MULTIMODULE_SUBTYPE_ROWS(EXTERNAL_MODULE)
  MULTIMODULE_STATUS_ROW
  EXTERNAL_MODULE_CHANNELS_ROWS,
  EXTERNAL_MODULE_BIND_ROWS(),
  OUTPUT_TYPE_ROWS()
  FAILSAFE_ROWS(EXTERNAL_MODULE),
  MULTIMODULE_MODULE_ROWS
  EXTRA_MODULE_ROWS
  TRAINER_MODULE_ROWS });
#elif defined(CPUM64)
  uint8_t protocol = g_model.protocol;
  MENU_TAB({ HEADER_LINE_COLUMNS 0, 2, CASE_PERSISTENT_TIMERS(0) 0, 0, 2, CASE_PERSISTENT_TIMERS(0) 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, NUM_STICKS+NUM_POTS+NUM_SLIDERS+NUM_ROTARY_ENCODERS-1, FIELD_PROTOCOL_MAX, 2 });
#else
  uint8_t protocol = g_model.protocol;
  MENU_TAB({ HEADER_LINE_COLUMNS 0, 2, CASE_PERSISTENT_TIMERS(0) 0, 0, 2, CASE_PERSISTENT_TIMERS(0) 0, 0, 0, 1, 0, 0, 0, 0, 0, NUM_SWITCHES, NUM_STICKS+NUM_POTS+NUM_SLIDERS+NUM_ROTARY_ENCODERS-1, FIELD_PROTOCOL_MAX, 2, CASE_PCBSKY9X(1) CASE_PCBSKY9X(2) });
#endif

  MENU_CHECK(menuTabModel, MENU_MODEL_SETUP, MODEL_SETUP_MAX_LINES);

#if defined(CPUARM) && (defined(DSM2) || defined(PXX))
  if (menuEvent) {
    moduleFlag[0] = 0;
#if NUM_MODULES > 1
    moduleFlag[1] = 0;
#endif
  }
#endif

  TITLE(STR_MENUSETUP);

  uint8_t sub = menuVerticalPosition - HEADER_LINE;
  int8_t editMode = s_editMode;

  for (uint8_t i=0; i<NUM_BODY_LINES; ++i) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
    uint8_t k = i+menuVerticalOffset;
#if defined(CPUARM)
    for (int j=0; j<=k; j++) {
      if (mstate_tab[j+HEADER_LINE] == HIDDEN_ROW) {
        if (++k >= (int)DIM(mstate_tab)) {
    	  return;
    	}
      }
    }
#endif

    LcdFlags blink = ((editMode>0) ? BLINK|INVERS : INVERS);
    LcdFlags attr = (sub == k ? blink : 0);

    switch (k) {
      case ITEM_MODEL_NAME:
        editSingleName(MODEL_SETUP_2ND_COLUMN, y, STR_MODELNAME, g_model.header.name, sizeof(g_model.header.name), event, attr);
#if defined(CPUARM)
        memcpy(modelHeaders[g_eeGeneral.currModel].name, g_model.header.name, sizeof(g_model.header.name));
#endif
        break;

#if defined(CPUARM)
      case ITEM_MODEL_TIMER1:
      case ITEM_MODEL_TIMER2:
      case ITEM_MODEL_TIMER3:
      {
        unsigned int timerIdx = (k>=ITEM_MODEL_TIMER3 ? 2 : (k>=ITEM_MODEL_TIMER2 ? 1 : 0));
        TimerData * timer = &g_model.timers[timerIdx];
        drawStringWithIndex(0*FW, y, STR_TIMER, timerIdx+1);
        drawTimerMode(MODEL_SETUP_2ND_COLUMN, y, timer->mode, menuHorizontalPosition==0 ? attr : 0);
        drawTimer(MODEL_SETUP_2ND_COLUMN+5*FW-2+5*FWNUM+1, y, timer->start, RIGHT | (menuHorizontalPosition==1 ? attr : 0), menuHorizontalPosition==2 ? attr : 0);
        if (attr && (editMode>0 || p1valdiff)) {
          div_t qr = div(timer->start, 60);
          switch (menuHorizontalPosition) {
            case 0:
            {
              int8_t timerMode = timer->mode;
              if (timerMode < 0) timerMode -= TMRMODE_COUNT-1;
              CHECK_INCDEC_MODELVAR_CHECK(event, timerMode, -TMRMODE_COUNT-SWSRC_LAST+1, TMRMODE_COUNT+SWSRC_LAST-1, isSwitchAvailableInTimers);
              if (timerMode < 0) timerMode += TMRMODE_COUNT-1;
              timer->mode = timerMode;
#if defined(AUTOSWITCH)
              if (s_editMode>0) {
                int8_t val = timer->mode - (TMRMODE_COUNT-1);
                int8_t switchVal = checkIncDecMovedSwitch(val);
                if (val != switchVal) {
                  timer->mode = switchVal + (TMRMODE_COUNT-1);
                  storageDirty(EE_MODEL);
                }
              }
#endif
              break;
            }
            case 1:
              CHECK_INCDEC_MODELVAR_ZERO(event, qr.quot, 539); // 8:59
              timer->start = qr.rem + qr.quot*60;
              break;
            case 2:
              qr.rem -= checkIncDecModel(event, qr.rem+2, 1, 62)-2;
              timer->start -= qr.rem ;
              if ((int16_t)timer->start < 0) timer->start=0;
              if ((int16_t)timer->start > 5999) timer->start=32399; // 8:59:59
              break;
          }
        }
        break;
      }

      case ITEM_MODEL_TIMER1_NAME:
      case ITEM_MODEL_TIMER2_NAME:
      case ITEM_MODEL_TIMER3_NAME:
      {
        TimerData * timer = &g_model.timers[k>=ITEM_MODEL_TIMER3 ? 2 : (k>=ITEM_MODEL_TIMER2 ? 1 : 0)];
        editSingleName(MODEL_SETUP_2ND_COLUMN, y, STR_TIMER_NAME, timer->name, sizeof(timer->name), event, attr);
        break;
      }

      case ITEM_MODEL_TIMER1_MINUTE_BEEP:
      case ITEM_MODEL_TIMER2_MINUTE_BEEP:
      case ITEM_MODEL_TIMER3_MINUTE_BEEP:
      {
        TimerData * timer = &g_model.timers[k>=ITEM_MODEL_TIMER3 ? 2 : (k>=ITEM_MODEL_TIMER2 ? 1 : 0)];
        timer->minuteBeep = editCheckBox(timer->minuteBeep, MODEL_SETUP_2ND_COLUMN, y, STR_MINUTEBEEP, attr, event);
        break;
      }

      case ITEM_MODEL_TIMER1_COUNTDOWN_BEEP:
      case ITEM_MODEL_TIMER2_COUNTDOWN_BEEP:
      case ITEM_MODEL_TIMER3_COUNTDOWN_BEEP:
      {
        TimerData * timer = &g_model.timers[k>=ITEM_MODEL_TIMER3 ? 2 : (k>=ITEM_MODEL_TIMER2 ? 1 : 0)];
        timer->countdownBeep = editChoice(MODEL_SETUP_2ND_COLUMN, y, STR_BEEPCOUNTDOWN, STR_VBEEPCOUNTDOWN, timer->countdownBeep, COUNTDOWN_SILENT, COUNTDOWN_COUNT-1, attr, event);
        break;
      }

      case ITEM_MODEL_TIMER1_PERSISTENT:
      case ITEM_MODEL_TIMER2_PERSISTENT:
      case ITEM_MODEL_TIMER3_PERSISTENT:
      {
        TimerData * timer = &g_model.timers[k>=ITEM_MODEL_TIMER3 ? 2 : (k>=ITEM_MODEL_TIMER2 ? 1 : 0)];
        timer->persistent = editChoice(MODEL_SETUP_2ND_COLUMN, y, STR_PERSISTENT, STR_VPERSISTENT, timer->persistent, 0, 2, attr, event);
        break;
      }
#else
      case ITEM_MODEL_TIMER1:
      case ITEM_MODEL_TIMER2:
      case ITEM_MODEL_TIMER1_MINUTE_BEEP:
      case ITEM_MODEL_TIMER2_MINUTE_BEEP:
      case ITEM_MODEL_TIMER1_COUNTDOWN_BEEP:
      case ITEM_MODEL_TIMER2_COUNTDOWN_BEEP:
      {
        TimerData *timer = &g_model.timers[k>=ITEM_MODEL_TIMER2 ? 1 : 0];
        if (k==ITEM_MODEL_TIMER1_MINUTE_BEEP || k==ITEM_MODEL_TIMER2_MINUTE_BEEP) {
          timer->minuteBeep = editCheckBox(timer->minuteBeep, MODEL_SETUP_2ND_COLUMN, y, STR_MINUTEBEEP, attr, event);
        }
        else if (k==ITEM_MODEL_TIMER1_COUNTDOWN_BEEP || k==ITEM_MODEL_TIMER2_COUNTDOWN_BEEP) {
          timer->countdownBeep = editCheckBox(timer->countdownBeep, MODEL_SETUP_2ND_COLUMN, y, STR_BEEPCOUNTDOWN, attr, event);
        }
        else {
          drawStringWithIndex(0*FW, y, STR_TIMER, k>=ITEM_MODEL_TIMER2 ? 2 : 1);
          drawTimerMode(MODEL_SETUP_2ND_COLUMN, y, timer->mode, menuHorizontalPosition==0 ? attr : 0);
          drawTimer(MODEL_SETUP_2ND_COLUMN+5*FW-2+5*FWNUM+1, y, timer->start, menuHorizontalPosition==1 ? attr : 0, menuHorizontalPosition==2 ? attr : 0);
          if (attr && (editMode>0 || p1valdiff)) {
            div_t qr = div(timer->start, 60);
            switch (menuHorizontalPosition) {
              case 0:
                CHECK_INCDEC_MODELVAR_CHECK(event, timer->mode, SWSRC_FIRST, TMRMODE_COUNT+SWSRC_LAST-1/*SWSRC_None removed*/, isSwitchAvailableInTimers);
                break;
              case 1:
                CHECK_INCDEC_MODELVAR_ZERO(event, qr.quot, 59);
                timer->start = qr.rem + qr.quot*60;
                break;
              case 2:
                qr.rem -= checkIncDecModel(event, qr.rem+2, 1, 62) - 2;
                if ((int16_t)timer->start >= qr.rem) {
                  timer->start -= qr.rem ;
                }
                if ((int16_t)timer->start > 3599) {
                  timer->start = 3599; // 59:59
                }
                break;
            }
          }
        }
        break;
      }

#if defined(CPUM2560)
      case ITEM_MODEL_TIMER1_PERSISTENT:
      case ITEM_MODEL_TIMER2_PERSISTENT:
      {
        TimerData &timer = g_model.timers[k==ITEM_MODEL_TIMER2_PERSISTENT];
        timer.persistent = editChoice(MODEL_SETUP_2ND_COLUMN, y, STR_PERSISTENT, STR_VPERSISTENT, timer.persistent, 0, 2, attr, event);
        break;
      }
#endif
#endif

      case ITEM_MODEL_EXTENDED_LIMITS:
        ON_OFF_MENU_ITEM(g_model.extendedLimits, MODEL_SETUP_2ND_COLUMN, y, STR_ELIMITS, attr, event);
        break;

      case ITEM_MODEL_EXTENDED_TRIMS:
#if defined(CPUM64)
        ON_OFF_MENU_ITEM(g_model.extendedTrims, MODEL_SETUP_2ND_COLUMN, y, STR_ETRIMS, attr, event);
#else
        ON_OFF_MENU_ITEM(g_model.extendedTrims, MODEL_SETUP_2ND_COLUMN, y, STR_ETRIMS, menuHorizontalPosition<=0 ? attr : 0, event==EVT_KEY_BREAK(KEY_ENTER) ? event : 0);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN+4*FW, y, STR_RESET_BTN, (menuHorizontalPosition>0  && !NO_HIGHLIGHT()) ? attr : 0);
        if (attr && menuHorizontalPosition>0) {
          s_editMode = 0;
          if (event==EVT_KEY_LONG(KEY_ENTER)) {
            START_NO_HIGHLIGHT();
            for (uint8_t i=0; i<MAX_FLIGHT_MODES; i++) {
              memclear(&g_model.flightModeData[i], TRIMS_ARRAY_SIZE);
            }
            storageDirty(EE_MODEL);
            AUDIO_WARNING1();
          }
        }
#endif
        break;

#if defined(CPUARM)
      case ITEM_MODEL_DISPLAY_TRIMS:
        g_model.displayTrims = editChoice(MODEL_SETUP_2ND_COLUMN, y, STR_DISPLAY_TRIMS, STR_VDISPLAYTRIMS, g_model.displayTrims, 0, 2, attr, event);
        break;
#endif

      case ITEM_MODEL_TRIM_INC:
        g_model.trimInc = editChoice(MODEL_SETUP_2ND_COLUMN, y, STR_TRIMINC, STR_VTRIMINC, g_model.trimInc, -2, 2, attr, event);
        break;

      case ITEM_MODEL_THROTTLE_REVERSED:
        ON_OFF_MENU_ITEM(g_model.throttleReversed, MODEL_SETUP_2ND_COLUMN, y, STR_THROTTLEREVERSE, attr, event ) ;
        break;

      case ITEM_MODEL_THROTTLE_TRACE:
      {
        lcdDrawTextAlignedLeft(y, STR_TTRACE);
        if (attr) CHECK_INCDEC_MODELVAR_ZERO(event, g_model.thrTraceSrc, NUM_POTS+NUM_SLIDERS+MAX_OUTPUT_CHANNELS);
        uint8_t idx = g_model.thrTraceSrc + MIXSRC_Thr;
        if (idx > MIXSRC_Thr)
          idx += 1;
        if (idx >= MIXSRC_FIRST_POT+NUM_POTS+NUM_SLIDERS)
          idx += MIXSRC_CH1 - MIXSRC_FIRST_POT - NUM_POTS - NUM_SLIDERS;
        drawSource(MODEL_SETUP_2ND_COLUMN, y, idx, attr);
        break;
      }

      case ITEM_MODEL_THROTTLE_TRIM:
        ON_OFF_MENU_ITEM(g_model.thrTrim, MODEL_SETUP_2ND_COLUMN, y, STR_TTRIM, attr, event);
        break;

#if defined(CPUARM)
      case ITEM_MODEL_PREFLIGHT_LABEL:
        lcdDrawTextAlignedLeft(y, STR_PREFLIGHT);
        break;

      case ITEM_MODEL_CHECKLIST_DISPLAY:
        ON_OFF_MENU_ITEM(g_model.displayChecklist, MODEL_SETUP_2ND_COLUMN, y, STR_CHECKLIST, attr, event);
        break;
#endif

      case ITEM_MODEL_THROTTLE_WARNING:
        g_model.disableThrottleWarning = !editCheckBox(!g_model.disableThrottleWarning, MODEL_SETUP_2ND_COLUMN, y, STR_THROTTLEWARNING, attr, event);
        break;

      case ITEM_MODEL_SWITCHES_WARNING:
      {
        lcdDrawTextAlignedLeft(y, STR_SWITCHWARNING);
        swarnstate_t states = g_model.switchWarningState;
        char c;
        if (attr) {
          s_editMode = 0;
          if (!READ_ONLY()) {
            switch (event) {
              CASE_EVT_ROTARY_BREAK
              case EVT_KEY_BREAK(KEY_ENTER):
#if defined(CPUM64)
                g_model.switchWarningEnable ^= (1 << menuHorizontalPosition);
                storageDirty(EE_MODEL);
#else
                if (menuHorizontalPosition < NUM_SWITCHES-1) {
                  g_model.switchWarningEnable ^= (1 << menuHorizontalPosition);
                  storageDirty(EE_MODEL);
                }
#endif
                break;

              case EVT_KEY_LONG(KEY_ENTER):
#if defined(CPUM64)
                getMovedSwitch();
                g_model.switchWarningState = switches_states;
                AUDIO_WARNING1();
                storageDirty(EE_MODEL);
#else
                if (menuHorizontalPosition == NUM_SWITCHES-1) {
                  START_NO_HIGHLIGHT();
                  getMovedSwitch();
                  g_model.switchWarningState = switches_states;
                  AUDIO_WARNING1();
                  storageDirty(EE_MODEL);
                }
#endif
                killEvents(event);
                break;
            }
          }
        }

        LcdFlags line = attr;

        for (uint8_t i=0; i<NUM_SWITCHES-1/*not on TRN switch*/; i++) {
          uint8_t swactive = !(g_model.switchWarningEnable & 1 << i);
          attr = 0;

          if (IS_3POS(i)) {
            c = '0'+(states & 0x03);
            states >>= 2;
          }
          else {
            if ((states & 0x01) && swactive)
              attr = INVERS;
            c = pgm_read_byte(STR_VSWITCHES - 2 + 9 + (3*(i+1)));
            states >>= 1;
          }
          if (line && (menuHorizontalPosition == i)) {
            attr = BLINK;
            if (swactive)
              attr |= INVERS;
          }
          lcdDrawChar(MODEL_SETUP_2ND_COLUMN+i*FW, y, (swactive || (attr & BLINK)) ? c : '-', attr);
#if !defined(CPUM64)
          lcdDrawText(MODEL_SETUP_2ND_COLUMN+(NUM_SWITCHES*FW), y, PSTR("<]"), (menuHorizontalPosition == NUM_SWITCHES-1 && !NO_HIGHLIGHT()) ? line : 0);
#endif
        }
        break;
      }

      case ITEM_MODEL_BEEP_CENTER:
        lcdDrawTextAlignedLeft(y, STR_BEEPCTR);
        for (uint8_t i=0; i<NUM_STICKS+NUM_POTS+NUM_SLIDERS+NUM_ROTARY_ENCODERS; i++) {
          // TODO flash saving, \001 not needed in STR_RETA123
          coord_t x = MODEL_SETUP_2ND_COLUMN+i*FW;
          lcdDrawTextAtIndex(x, y, STR_RETA123, i, ((menuHorizontalPosition==i) && attr) ? BLINK|INVERS : (((g_model.beepANACenter & ((BeepANACenter)1<<i)) || (attr && CURSOR_ON_LINE())) ? INVERS : 0 ) );
        }
        if (attr && CURSOR_ON_CELL) {
          if (event==EVT_KEY_BREAK(KEY_ENTER) || p1valdiff) {
            if (READ_ONLY_UNLOCKED()) {
              s_editMode = 0;
              g_model.beepANACenter ^= ((BeepANACenter)1<<menuHorizontalPosition);
              storageDirty(EE_MODEL);
            }
          }
        }
        break;

#if defined(CPUARM)
      case ITEM_MODEL_USE_GLOBAL_FUNCTIONS:
        lcdDrawTextAlignedLeft(y, STR_USE_GLOBAL_FUNCS);
        drawCheckBox(MODEL_SETUP_2ND_COLUMN, y, !g_model.noGlobalFunctions, attr);
        if (attr) g_model.noGlobalFunctions = !checkIncDecModel(event, !g_model.noGlobalFunctions, 0, 1);
        break;
#endif

#if defined(PCBX7)
      case ITEM_MODEL_INTERNAL_MODULE_LABEL:
        lcdDrawTextAlignedLeft(y, TR_INTERNALRF);
        break;
        
      case ITEM_MODEL_INTERNAL_MODULE_MODE:
        lcdDrawTextAlignedLeft(y, STR_MODE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_XJT_PROTOCOLS, 1+g_model.moduleData[0].rfProtocol, attr);
        if (attr) {
          g_model.moduleData[INTERNAL_MODULE].rfProtocol = checkIncDec(event, g_model.moduleData[INTERNAL_MODULE].rfProtocol, RF_PROTO_OFF, RF_PROTO_LAST, EE_MODEL, isRfProtocolAvailable);
          if (checkIncDec_Ret) {
            g_model.moduleData[0].type = MODULE_TYPE_XJT;
            g_model.moduleData[0].channelsStart = 0;
            g_model.moduleData[0].channelsCount = DEFAULT_CHANNELS(INTERNAL_MODULE);
          }
        }
        break;
#endif

#if defined(PCBSKY9X)
      case ITEM_MODEL_EXTRA_MODULE_LABEL:
        lcdDrawTextAlignedLeft(y, "RF Port 2 (PPM)");
        break;
#endif

#if defined(CPUARM)
      case ITEM_MODEL_EXTERNAL_MODULE_LABEL:
        lcdDrawTextAlignedLeft(y, TR_EXTERNALRF);
        break;

      case ITEM_MODEL_EXTERNAL_MODULE_MODE:
        lcdDrawTextAlignedLeft(y, STR_MODE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_TARANIS_PROTOCOLS, g_model.moduleData[EXTERNAL_MODULE].type, menuHorizontalPosition==0 ? attr : 0);
        if (IS_MODULE_XJT(EXTERNAL_MODULE))
          lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN+5*FW, y, STR_XJT_PROTOCOLS, 1+g_model.moduleData[EXTERNAL_MODULE].rfProtocol, menuHorizontalPosition==1 ? attr : 0);
        else if (IS_MODULE_DSM2(EXTERNAL_MODULE))
          lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN+5*FW, y, STR_DSM_PROTOCOLS, g_model.moduleData[EXTERNAL_MODULE].rfProtocol, menuHorizontalPosition==1 ? attr : 0);
#if defined(MULTIMODULE)
        else if (IS_MODULE_MULTIMODULE(EXTERNAL_MODULE)) {
          int multi_rfProto = g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false);
          if (g_model.moduleData[EXTERNAL_MODULE].multi.customProto)
            lcdDrawText(MODEL_SETUP_2ND_COLUMN+5*FW, y, STR_MULTI_CUSTOM, menuHorizontalPosition==1 ? attr : 0);
          else
            lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN+5*FW, y, STR_MULTI_PROTOCOLS, multi_rfProto, menuHorizontalPosition==1 ? attr : 0);
        }
#endif
        if (attr && (editMode>0 || p1valdiff)) {
          switch (menuHorizontalPosition) {
            case 0:
              g_model.moduleData[EXTERNAL_MODULE].type = checkIncDec(event, g_model.moduleData[EXTERNAL_MODULE].type, MODULE_TYPE_NONE, IS_TRAINER_EXTERNAL_MODULE() ? MODULE_TYPE_NONE : MODULE_TYPE_COUNT-1, EE_MODEL, isModuleAvailable);
              if (checkIncDec_Ret) {
                g_model.moduleData[EXTERNAL_MODULE].rfProtocol = 0;
                g_model.moduleData[EXTERNAL_MODULE].channelsStart = 0;
                g_model.moduleData[EXTERNAL_MODULE].channelsCount = DEFAULT_CHANNELS(EXTERNAL_MODULE);
              }
              break;
            case 1:
              if (IS_MODULE_DSM2(EXTERNAL_MODULE))
                CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].rfProtocol, DSM2_PROTO_LP45, DSM2_PROTO_DSMX);
#if defined(MULTIMODULE)
              else if (IS_MODULE_MULTIMODULE(EXTERNAL_MODULE)) {
                int multiRfProto = g_model.moduleData[EXTERNAL_MODULE].multi.customProto == 1 ? MM_RF_PROTO_CUSTOM : g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false);
                CHECK_INCDEC_MODELVAR(event, multiRfProto, MM_RF_PROTO_FIRST, MM_RF_PROTO_LAST);
                if (checkIncDec_Ret) {
                  g_model.moduleData[EXTERNAL_MODULE].multi.customProto = (multiRfProto == MM_RF_PROTO_CUSTOM);
                  if (!g_model.moduleData[EXTERNAL_MODULE].multi.customProto)
                    g_model.moduleData[EXTERNAL_MODULE].setMultiProtocol(multiRfProto);
                  g_model.moduleData[EXTERNAL_MODULE].subType = 0;
                  // Sensible default for DSM2 (same as for ppm): 7ch@22ms + Autodetect settings enabled
                  if (g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(true) == MM_RF_PROTO_DSM2) {
                    g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode = 1;
                  } else {
                    g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode = 0;
                  }
                  g_model.moduleData[EXTERNAL_MODULE].multi.optionValue = 0;
                }
              }
#endif
              else {
                CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].rfProtocol, RF_PROTO_X16, RF_PROTO_LAST);
              }
              if (checkIncDec_Ret) {
                g_model.moduleData[EXTERNAL_MODULE].channelsStart = 0;
                g_model.moduleData[EXTERNAL_MODULE].channelsCount = DEFAULT_CHANNELS(EXTERNAL_MODULE);
              }
          }
        }
        break;
#endif

#if defined(MULTIMODULE)
      case ITEM_MODEL_EXTERNAL_MODULE_SUBTYPE:
      {
        lcdDrawTextAlignedLeft(y, STR_SUBTYPE);
        uint8_t multi_rfProto = g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(true);
        const mm_protocol_definition *pdef = getMultiProtocolDefinition(multi_rfProto);

        if (multi_rfProto == MM_RF_CUSTOM_SELECTED) {
            lcdDrawNumber(MODEL_SETUP_2ND_COLUMN + 3 * FW, y, g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false), RIGHT | (menuHorizontalPosition == 0 ? attr : 0), 2);
            lcdDrawNumber(MODEL_SETUP_2ND_COLUMN + 5 * FW, y, g_model.moduleData[EXTERNAL_MODULE].subType, RIGHT | (menuHorizontalPosition == 1 ? attr : 0), 2);
        } else
        {
          if (pdef->subTypeString != nullptr)
            lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, pdef->subTypeString, g_model.moduleData[EXTERNAL_MODULE].subType, attr);
        }
        if (attr && (editMode > 0 || p1valdiff)) {
          switch (menuHorizontalPosition) {
            case 0:
              if (multi_rfProto == MM_RF_CUSTOM_SELECTED)
                g_model.moduleData[EXTERNAL_MODULE].setMultiProtocol(checkIncDec(event, g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false), 0, 63, EE_MODEL));
              else if (pdef->maxSubtype > 0)
                CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].subType, 0, pdef->maxSubtype);

            case 1:
              // Custom protocol, third column is subtype
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].subType, 0, 7);
              break;
          }
        }
      }
        break;
#endif

#if defined(PCBX7)
      case ITEM_MODEL_TRAINER_LABEL:
        lcdDrawTextAlignedLeft(y, STR_TRAINER);
        break;
        
      case ITEM_MODEL_TRAINER_MODE:
        lcdDrawTextAlignedLeft(y, STR_MODE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_VTRAINERMODES, g_model.trainerMode, attr);
        if (attr) g_model.trainerMode = checkIncDec(event, g_model.trainerMode, 0, HAS_WIRELESS_TRAINER_HARDWARE() ? TRAINER_MODE_MASTER_BATTERY_COMPARTMENT : TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE, EE_MODEL, isTrainerModeAvailable);
        break;
#endif

#if defined(PCBX7)
      case ITEM_MODEL_INTERNAL_MODULE_CHANNELS:
      case ITEM_MODEL_TRAINER_CHANNELS:
#endif
#if defined(PCBSKY9X)
      case ITEM_MODEL_EXTRA_MODULE_CHANNELS:
#endif
#if defined(CPUARM)
      case ITEM_MODEL_EXTERNAL_MODULE_CHANNELS:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(k);
        ModuleData & moduleData = g_model.moduleData[moduleIdx];
        lcdDrawTextAlignedLeft(y, STR_CHANNELRANGE);
        if ((int8_t)PORT_CHANNELS_ROWS(moduleIdx) >= 0) {
          lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, STR_CH, menuHorizontalPosition==0 ? attr : 0);
          lcdDrawNumber(lcdLastPos, y, moduleData.channelsStart+1, LEFT | (menuHorizontalPosition==0 ? attr : 0));
          lcdDrawChar(lcdLastPos, y, '-');
          lcdDrawNumber(lcdLastPos + FW+1, y, moduleData.channelsStart+NUM_CHANNELS(moduleIdx), LEFT | (menuHorizontalPosition==1 ? attr : 0));
          if (attr && (editMode>0 || p1valdiff)) {
            switch (menuHorizontalPosition) {
              case 0:
                CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.channelsStart, 32-8-moduleData.channelsCount);
                break;
              case 1:
                CHECK_INCDEC_MODELVAR(event, moduleData.channelsCount, -4, min<int8_t>(MAX_CHANNELS(moduleIdx), 32-8-moduleData.channelsStart));
                if ((k == ITEM_MODEL_EXTERNAL_MODULE_CHANNELS && g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_PPM)) {
                  SET_DEFAULT_PPM_FRAME_LENGTH(moduleIdx);
                }
                break;
            }
          }
        }
        break;
      }
#endif

#if defined(PCBX7)
      case ITEM_MODEL_TRAINER_SETTINGS:
      case ITEM_MODEL_INTERNAL_MODULE_BIND:
#endif
#if defined(PCBSKY9X)
      case ITEM_MODEL_EXTRA_MODULE_BIND:
#endif
#if defined(CPUARM)
      case ITEM_MODEL_EXTERNAL_MODULE_BIND:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(k);
        ModuleData & moduleData = g_model.moduleData[moduleIdx];
        if (IS_MODULE_PPM(moduleIdx)) {
          lcdDrawTextAlignedLeft(y, STR_PPMFRAME);
          lcdDrawText(MODEL_SETUP_2ND_COLUMN+3*FW, y, STR_MS);
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, (int16_t)moduleData.ppm.frameLength*5 + 225, (menuHorizontalPosition<=0 ? attr : 0) | PREC1|LEFT);
          lcdDrawChar(MODEL_SETUP_2ND_COLUMN+8*FW+2, y, 'u');
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN+8*FW+2, y, (moduleData.ppm.delay*50)+300, RIGHT | ((CURSOR_ON_LINE() || menuHorizontalPosition==1) ? attr : 0));
          lcdDrawChar(MODEL_SETUP_2ND_COLUMN+10*FW, y, moduleData.ppm.pulsePol ? '+' : '-', (CURSOR_ON_LINE() || menuHorizontalPosition==2) ? attr : 0);
          if (attr && (editMode>0 || p1valdiff)) {
            switch (menuHorizontalPosition) {
              case 0:
                CHECK_INCDEC_MODELVAR(event, moduleData.ppm.frameLength, -20, 35);
                break;
              case 1:
                CHECK_INCDEC_MODELVAR(event, moduleData.ppm.delay, -4, 10);
                break;
              case 2:
                CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.ppm.pulsePol, 1);
                break;
            }
          }
        }
        else {
          horzpos_t l_posHorz = menuHorizontalPosition;
          coord_t xOffsetBind = MODEL_SETUP_BIND_OFS;
          if (IS_MODULE_XJT(moduleIdx) && IS_D8_RX(moduleIdx)) {
            xOffsetBind = 0;
            lcdDrawTextAlignedLeft(y, STR_RECEIVER);
            if (attr) l_posHorz += 1;
          }
          else {
            lcdDrawTextAlignedLeft(y, STR_RECEIVER_NUM);
          }
          if (IS_MODULE_XJT(moduleIdx) || IS_MODULE_DSM2(moduleIdx) || IS_MODULE_MULTIMODULE(moduleIdx)) {
            if (xOffsetBind) lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, g_model.header.modelId[moduleIdx], (l_posHorz==0 ? attr : 0) | LEADING0|LEFT, 2);
            if (attr && l_posHorz==0) {
              if (editMode>0 || p1valdiff) {
                CHECK_INCDEC_MODELVAR_ZERO(event, g_model.header.modelId[moduleIdx], MAX_RX_NUM(moduleIdx));
                if (checkIncDec_Ret) {
                  modelHeaders[g_eeGeneral.currModel].modelId[moduleIdx] = g_model.header.modelId[moduleIdx];
                }
              }
              if (editMode==0 && event==EVT_KEY_BREAK(KEY_ENTER)) {
                checkModelIdUnique(g_eeGeneral.currModel, moduleIdx);
              }
            }
            lcdDrawText(MODEL_SETUP_2ND_COLUMN+xOffsetBind, y, STR_MODULE_BIND, l_posHorz==1 ? attr : 0);
            lcdDrawText(MODEL_SETUP_2ND_COLUMN+MODEL_SETUP_RANGE_OFS+xOffsetBind, y, STR_MODULE_RANGE, l_posHorz==2 ? attr : 0);
            uint8_t newFlag = 0;
#if defined(MULTIMODULE)
            if (multiBindStatus == MULTI_BIND_FINISHED) {
              multiBindStatus = MULTI_NORMAL_OPERATION;
              s_editMode=0;
            }
#endif
            if (attr && l_posHorz>0 && s_editMode>0) {
              if (l_posHorz == 1)
                newFlag = MODULE_BIND;
              else if (l_posHorz == 2) {
                newFlag = MODULE_RANGECHECK;
              }
            }
            moduleFlag[moduleIdx] = newFlag;
#if defined(MULTIMODULE)
            if (newFlag == MODULE_BIND)
              multiBindStatus = MULTI_BIND_INITIATED;
#endif
          }
        }
        break;
      }
#endif

#if defined(PCBSKY9X) && defined(REVX)
      case ITEM_MODEL_EXTERNAL_MODULE_OUTPUT_TYPE:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(k);
        ModuleData & moduleData = g_model.moduleData[moduleIdx];
        moduleData.ppm.outputType = editChoice(MODEL_SETUP_2ND_COLUMN, y, STR_OUTPUT_TYPE, STR_VOUTPUT_TYPE, moduleData.ppm.outputType, 0, 1, attr, event);
        break;
      }
#endif

#if defined(PCBX7)
      case ITEM_MODEL_INTERNAL_MODULE_FAILSAFE:
#endif
#if defined(CPUARM)
      case ITEM_MODEL_EXTERNAL_MODULE_FAILSAFE:
      {
        uint8_t moduleIdx = CURRENT_MODULE_EDITED(k);
        ModuleData & moduleData = g_model.moduleData[moduleIdx];
        if (IS_MODULE_XJT(moduleIdx)) {
          lcdDrawTextAlignedLeft(y, STR_FAILSAFE);
          lcdDrawTextAlignedLeft(y, TR_FAILSAFE);
          lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_VFAILSAFE, moduleData.failsafeMode, menuHorizontalPosition == 0
                                                                                                ? attr : 0);
          if (moduleData.failsafeMode == FAILSAFE_CUSTOM)
            lcdDrawText(MODEL_SETUP_2ND_COLUMN + MODEL_SETUP_SET_FAILSAFE_OFS, y, STR_SET, menuHorizontalPosition == 1
                                                                                           ? attr : 0);
          if (attr) {
            if (moduleData.failsafeMode != FAILSAFE_CUSTOM)
              menuHorizontalPosition = 0;
            if (menuHorizontalPosition == 0) {
              if (editMode > 0 || p1valdiff) {
                CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.failsafeMode, FAILSAFE_LAST);
                if (checkIncDec_Ret) SEND_FAILSAFE_NOW(moduleIdx);
              }
            }
            else if (menuHorizontalPosition == 1) {
              s_editMode = 0;
              if (moduleData.failsafeMode == FAILSAFE_CUSTOM && event == EVT_KEY_FIRST(KEY_ENTER)) {
                g_moduleIdx = moduleIdx;
                pushMenu(menuModelFailsafe);
              }
            }
            else {
              lcdDrawSolidFilledRect(MODEL_SETUP_2ND_COLUMN, y, LCD_W - MODEL_SETUP_2ND_COLUMN, 8);
            }
          }
        }
#if defined(MULTIMODULE)
        else if (IS_MODULE_MULTIMODULE(moduleIdx)) {
          int optionValue =  g_model.moduleData[moduleIdx].multi.optionValue;

          const uint8_t multi_proto = g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(false);
          const mm_protocol_definition* pdef = getMultiProtocolDefinition(multi_proto);
          if (pdef->optionsstr)
            lcdDrawTextAlignedLeft(y, pdef->optionsstr);

          if (multi_proto == MM_RF_PROTO_FS_AFHDS2A)
            optionValue = 50 + 5 * optionValue;

          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, optionValue, LEFT | attr);
          if (attr) {
            if (multi_proto == MM_RF_PROTO_FS_AFHDS2A) {
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].multi.optionValue, 0, 70);
            } else if (multi_proto == MM_RF_PROTO_OLRS) {
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].multi.optionValue, -1, 7);
            } else {
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].multi.optionValue, -128, 127);
            }
          }
        }
#endif
      }
      break;

#if defined(MULTIMODULE)
    case ITEM_MODEL_EXTERNAL_MODULE_AUTOBIND:
      if (g_model.moduleData[EXTERNAL_MODULE].getMultiProtocol(true) == MM_RF_PROTO_DSM2)
        g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode = editCheckBox(g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode, MODEL_SETUP_2ND_COLUMN, y, STR_MULTI_DSM_AUTODTECT, attr, event);
      else
        g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode = editCheckBox(g_model.moduleData[EXTERNAL_MODULE].multi.autoBindMode, MODEL_SETUP_2ND_COLUMN, y, STR_MULTI_AUTOBIND, attr, event);
      break;
    case  ITEM_MODEL_EXTERNAL_MODULE_LOWPOWER:
      g_model.moduleData[EXTERNAL_MODULE].multi.lowPowerMode = editCheckBox(g_model.moduleData[EXTERNAL_MODULE].multi.lowPowerMode, MODEL_SETUP_2ND_COLUMN, y, STR_MULTI_LOWPOWER, attr, event);
      break;
    case ITEM_MODEL_EXTERNAL_MODULE_STATUS: {
      lcdDrawTextAlignedLeft(y, STR_MODULE_STATUS);

      char statusText[64];
      multiModuleStatus.getStatusString(statusText);
      lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, statusText);
      break;
    }

#endif
#endif

#if !defined(CPUARM)
      case ITEM_MODEL_PPM1_PROTOCOL:
        lcdDrawTextAlignedLeft(y, NO_INDENT(STR_PROTO));
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_VPROTOS, protocol, menuHorizontalPosition<=0 ? attr : 0);
        if (IS_PPM_PROTOCOL(protocol)) {
          lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN+7*FW, y, STR_NCHANNELS, g_model.ppmNCH+2, menuHorizontalPosition!=0 ? attr : 0);
        }
        else if (menuHorizontalPosition>0 && attr) {
          MOVE_CURSOR_FROM_HERE();
        }
        if (attr && (editMode>0 || p1valdiff || (!IS_PPM_PROTOCOL(protocol) && !IS_DSM2_PROTOCOL(protocol)))) {
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.protocol, PROTO_MAX-1);
              break;
            case 1:
              CHECK_INCDEC_MODELVAR(event, g_model.ppmNCH, -2, 4);
              g_model.ppmFrameLength = g_model.ppmNCH * 8;
              break;
          }
        }
        break;
#endif

#if 0
      case ITEM_MODEL_PPM2_PROTOCOL:
        lcdDrawTextAlignedLeft(y, PSTR("Port2"));
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_VPROTOS, 0, 0);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN+4*FW+3, y, STR_CH, menuHorizontalPosition<=0 ? attr : 0);
        lcdDrawNumber(lcdLastPos, y, g_model.moduleData[1].channelsStart+1, LEFT | (menuHorizontalPosition<=0 ? attr : 0));
        lcdDrawChar(lcdLastPos, y, '-');
        lcdDrawNumber(lcdLastPos + FW+1, y, g_model.moduleData[1].channelsStart+8+g_model.moduleData[1].channelsCount, LEFT | (menuHorizontalPosition!=0 ? attr : 0));
        if (attr && (editMode>0 || p1valdiff)) {
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.moduleData[1].channelsStart, 32-8-g_model.moduleData[1].channelsCount);
              SET_DEFAULT_PPM_FRAME_LENGTH(1);
              break;
            case 1:
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[1].channelsCount, -4, min<int8_t>(8, 32-8-g_model.moduleData[1].channelsStart));
              SET_DEFAULT_PPM_FRAME_LENGTH(1);
              break;
          }
        }
        break;

      case ITEM_MODEL_PPM2_PARAMS:
        lcdDrawTextAlignedLeft(y, STR_PPMFRAME);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN+3*FW, y, STR_MS);
        lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, (int16_t)g_model.moduleData[1].ppmFrameLength*5 + 225, (menuHorizontalPosition<=0 ? attr : 0) | PREC1 | LEFT);
        lcdDrawChar(MODEL_SETUP_2ND_COLUMN+8*FW+2, y, 'u');
        lcdDrawNumber(MODEL_SETUP_2ND_COLUMN+8*FW+2, y, (g_model.moduleData[1].ppmDelay*50)+300, RIGHT | ((menuHorizontalPosition < 0 || menuHorizontalPosition==1) ? attr : 0));
        lcdDrawChar(MODEL_SETUP_2ND_COLUMN+10*FW, y, g_model.moduleData[1].ppmPulsePol ? '+' : '-', (menuHorizontalPosition < 0 || menuHorizontalPosition==2) ? attr : 0);
        if (attr && (editMode>0 || p1valdiff)) {
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[1].ppmFrameLength, -20, 35);
              break;
            case 1:
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[1].ppmDelay, -4, 10);
              break;
            case 2:
              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.moduleData[1].ppmPulsePol, 1);
              break;
          }
        }
        break;
#endif

#if !defined(CPUARM)
      case ITEM_MODEL_PPM1_PARAMS:
        if (IS_PPM_PROTOCOL(protocol)) {
          lcdDrawTextAlignedLeft(y, STR_PPMFRAME);
          lcdDrawText(MODEL_SETUP_2ND_COLUMN+3*FW, y, STR_MS);
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, (int16_t)g_model.ppmFrameLength*5 + 225, (menuHorizontalPosition<=0 ? attr : 0) | PREC1|LEFT);
          lcdDrawChar(MODEL_SETUP_2ND_COLUMN+8*FW+2, y, 'u');
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN+8*FW+2, y, (g_model.ppmDelay*50)+300, (CURSOR_ON_LINE() || menuHorizontalPosition==1) ? attr : 0);
          lcdDrawChar(MODEL_SETUP_2ND_COLUMN+10*FW, y, g_model.pulsePol ? '+' : '-', (CURSOR_ON_LINE() || menuHorizontalPosition==2) ? attr : 0);
          if (attr && (editMode>0 || p1valdiff)) {
            switch (menuHorizontalPosition) {
              case 0:
                CHECK_INCDEC_MODELVAR(event, g_model.ppmFrameLength, -20, 35);
                break;
              case 1:
                CHECK_INCDEC_MODELVAR(event, g_model.ppmDelay, -4, 10);
                break;
              case 2:
                CHECK_INCDEC_MODELVAR_ZERO(event, g_model.pulsePol, 1);
                break;
            }
          }
        }
#if defined(DSM2) || defined(PXX)
        else if (IS_DSM2_PROTOCOL(protocol) || IS_PXX_PROTOCOL(protocol)) {
          if (attr && menuHorizontalPosition > 1) {
            REPEAT_LAST_CURSOR_MOVE(); // limit 3 column row to 2 colums (Rx_Num and RANGE fields)
          }
          lcdDrawTextAlignedLeft(y, STR_RECEIVER_NUM);
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, g_model.header.modelId[0], (menuHorizontalPosition<=0 ? attr : 0) | LEADING0|LEFT, 2);
          if (attr && (menuHorizontalPosition==0 && (editMode>0 || p1valdiff))) {
            CHECK_INCDEC_MODELVAR_ZERO(event, g_model.header.modelId[0], 99);
          }
#if defined(PXX)
          if (protocol == PROTO_PXX) {
            lcdDrawText(MODEL_SETUP_2ND_COLUMN+4*FW, y, STR_SYNCMENU, menuHorizontalPosition!=0 ? attr : 0);
            uint8_t newFlag = 0;
            if (attr && menuHorizontalPosition>0 && editMode>0) {
              // send reset code
              newFlag = MODULE_BIND;
            }
            moduleFlag[0] = newFlag;
          }
#endif
#if defined(DSM2)
          if (IS_DSM2_PROTOCOL(protocol)) {
            lcdDrawText(MODEL_SETUP_2ND_COLUMN+4*FW, y, STR_MODULE_RANGE, menuHorizontalPosition!=0 ? attr : 0);
            moduleFlag[0] = (attr && menuHorizontalPosition>0 && editMode>0) ? MODULE_RANGECHECK : 0; // [MENU] key toggles range check mode
          }
#endif
        }
#endif
        break;
#endif
    }
  }

#if defined(CPUARM) && defined(PXX)
  if (IS_RANGECHECK_ENABLE()) {
    showMessageBox("RSSI: ");
    lcdDrawNumber(16+4*FW, 5*FH, TELEMETRY_RSSI(), BOLD);
  }
#endif
}

#if defined(CPUARM)
void menuModelFailsafe(event_t event)
{
  uint8_t ch = 8 * (menuVerticalPosition / 8);
  uint8_t channelStart = g_model.moduleData[g_moduleIdx].channelsStart;

  if (event == EVT_KEY_LONG(KEY_ENTER)) {
    killEvents(event);
    event = 0;
    if (s_editMode) {
      g_model.moduleData[g_moduleIdx].failsafeChannels[menuVerticalPosition] = channelOutputs[menuVerticalPosition+channelStart];
      storageDirty(EE_MODEL);
      AUDIO_WARNING1();
      s_editMode = 0;
      SEND_FAILSAFE_NOW(g_moduleIdx);
    }
    else {
      int16_t & failsafe = g_model.moduleData[g_moduleIdx].failsafeChannels[menuVerticalPosition];
      if (failsafe < FAILSAFE_CHANNEL_HOLD)
        failsafe = FAILSAFE_CHANNEL_HOLD;
      else if (failsafe == FAILSAFE_CHANNEL_HOLD)
        failsafe = FAILSAFE_CHANNEL_NOPULSE;
      else
        failsafe = 0;
      storageDirty(EE_MODEL);
      AUDIO_WARNING1();
      SEND_FAILSAFE_NOW(g_moduleIdx);
    }
  }

  SIMPLE_SUBMENU_NOTITLE(NUM_CHANNELS(g_moduleIdx));

  const uint8_t SLIDER_W = 90;

  lcdDrawTextAlignedCenter(0*FH, FAILSAFESET);
  lcdInvertLine(0);

  unsigned int lim = g_model.extendedLimits ? 640*2 : 512*2;

  coord_t x = 1;

  // Channels
  for (uint8_t line=0; line<8; line++) {
    coord_t y = 9+line*7;
    int32_t channelValue = channelOutputs[ch+channelStart];
    int32_t failsafeValue = 0;
    bool failsafeEditable = false;

    if (ch < NUM_CHANNELS(g_moduleIdx)) {
      failsafeValue = g_model.moduleData[g_moduleIdx].failsafeChannels[ch];
      failsafeEditable = true;
    }

    if (failsafeEditable) {
      //Channel
      putsChn(x+1, y, ch+1, SMLSIZE);

      // Value
      LcdFlags flags = TINSIZE;
      if (menuVerticalPosition == ch) {
        flags |= INVERS;
        if (s_editMode) {
          if (failsafeValue == FAILSAFE_CHANNEL_HOLD || failsafeValue == FAILSAFE_CHANNEL_NOPULSE) {
            s_editMode = 0;
          }
          else {
            flags |= BLINK;
            CHECK_INCDEC_MODELVAR(event, g_model.moduleData[g_moduleIdx].failsafeChannels[ch], -lim, +lim);
          }
        }
      }

#if defined(PPM_UNIT_PERCENT_PREC1)
      uint8_t wbar = SLIDER_W-6;
#else
      uint8_t wbar = SLIDER_W;
#endif

      uint8_t xValue = x+LCD_W-4-wbar;
      if (failsafeValue == FAILSAFE_CHANNEL_HOLD) {
        lcdDrawText(xValue, y, STR_HOLD, RIGHT|flags);
        failsafeValue = 0;
      }
      else if (failsafeValue == FAILSAFE_CHANNEL_NOPULSE) {
        lcdDrawText(xValue, y, STR_NONE, RIGHT|flags);
        failsafeValue = 0;
      }
      else {
#if defined(PPM_UNIT_US)
        lcdDrawNumber(xValue, y, PPM_CH_CENTER(ch)+failsafeValue/2, RIGHT|flags);
#elif defined(PPM_UNIT_PERCENT_PREC1)
        lcdDrawNumber(xValue, y, calcRESXto1000(failsafeValue), RIGHT|PREC1|flags);
#else
        lcdDrawNumber(xValue, y, calcRESXto1000(failsafeValue)/10, RIGHT|flags);
#endif
      }

      // Gauge
#if !defined(PCBX7)  // X7 LCD doesn't like too many horizontal lines
      lcdDrawRect(x+LCD_W-3-wbar, y, wbar+1, 6);
#endif
      unsigned int lenChannel = limit((uint8_t)1, uint8_t((abs(channelValue) * wbar/2 + lim/2) / lim), uint8_t(wbar/2));
      unsigned int lenFailsafe = limit((uint8_t)1, uint8_t((abs(failsafeValue) * wbar/2 + lim/2) / lim), uint8_t(wbar/2));
      coord_t xChannel = (channelValue>0) ? x+LCD_W-3-wbar/2 : x+LCD_W-2-wbar/2-lenChannel;
      coord_t xFailsafe = (failsafeValue>0) ? x+LCD_W-3-wbar/2 : x+LCD_W-2-wbar/2-lenFailsafe;
      lcdDrawHorizontalLine(xChannel, y+1, lenChannel, DOTTED, 0);
      lcdDrawHorizontalLine(xChannel, y+2, lenChannel, DOTTED, 0);
      lcdDrawSolidHorizontalLine(xFailsafe, y+3, lenFailsafe);
      lcdDrawSolidHorizontalLine(xFailsafe, y+4, lenFailsafe);
    }
    ch++;
  }
}
#endif