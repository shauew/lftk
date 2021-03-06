/**
 * File:   lcd.c
 * Author: Li XianJing <xianjimli@hotmail.com>
 * Brief:  lcd interface
 *
 * Copyright (c) 2018 - 2018  Li XianJing <xianjimli@hotmail.com>
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2018-01-13 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "base/lcd.h"

ret_t lcd_begin_frame(lcd_t* lcd, rect_t* dirty_rect, lcd_draw_mode_t draw_mode) {
  return_value_if_fail(lcd != NULL && lcd->begin_frame != NULL, RET_BAD_PARAMS);

  lcd->draw_mode = draw_mode;
  return lcd->begin_frame(lcd, dirty_rect);
}

ret_t lcd_set_clip_rect(lcd_t* lcd, rect_t* rect) {
  return_value_if_fail(lcd != NULL && lcd->set_clip_rect != NULL, RET_BAD_PARAMS);

  return lcd->set_clip_rect(lcd, rect);
}

ret_t lcd_set_global_alpha(lcd_t* lcd, uint8_t alpha) {
  return_value_if_fail(lcd != NULL, RET_BAD_PARAMS);

  lcd->global_alpha = alpha;
  if (lcd->set_global_alpha != NULL) {
    lcd->set_global_alpha(lcd, alpha);
  }

  return RET_OK;
}

ret_t lcd_set_text_color(lcd_t* lcd, color_t color) {
  return_value_if_fail(lcd != NULL, RET_BAD_PARAMS);

  lcd->text_color = color;
  if (lcd->set_text_color != NULL) {
    lcd->set_text_color(lcd, color);
  }

  return RET_OK;
}

ret_t lcd_set_stroke_color(lcd_t* lcd, color_t color) {
  return_value_if_fail(lcd != NULL, RET_BAD_PARAMS);

  lcd->stroke_color = color;
  if (lcd->set_stroke_color != NULL) {
    lcd->set_stroke_color(lcd, color);
  }

  return RET_OK;
}

ret_t lcd_set_fill_color(lcd_t* lcd, color_t color) {
  return_value_if_fail(lcd != NULL, RET_BAD_PARAMS);

  lcd->fill_color = color;
  if (lcd->set_fill_color != NULL) {
    lcd->set_fill_color(lcd, color);
  }

  return RET_OK;
}

ret_t lcd_set_font_name(lcd_t* lcd, const char* name) {
  return_value_if_fail(lcd != NULL, RET_BAD_PARAMS);

  lcd->font_name = name;
  if (lcd->set_font_name != NULL) {
    lcd->set_font_name(lcd, name);
  }

  return RET_OK;
}

ret_t lcd_set_font_size(lcd_t* lcd, uint32_t size) {
  return_value_if_fail(lcd != NULL, RET_BAD_PARAMS);

  lcd->font_size = size;
  if (lcd->set_font_size != NULL) {
    lcd->set_font_size(lcd, size);
  }

  return RET_OK;
}

ret_t lcd_draw_vline(lcd_t* lcd, xy_t x, xy_t y, wh_t h) {
  return_value_if_fail(lcd != NULL && lcd->draw_vline != NULL, RET_BAD_PARAMS);

  return lcd->draw_vline(lcd, x, y, h);
}

ret_t lcd_draw_hline(lcd_t* lcd, xy_t x, xy_t y, wh_t w) {
  return_value_if_fail(lcd != NULL && lcd->draw_hline != NULL, RET_BAD_PARAMS);

  return lcd->draw_hline(lcd, x, y, w);
}

ret_t lcd_fill_rect(lcd_t* lcd, xy_t x, xy_t y, wh_t w, wh_t h) {
  return_value_if_fail(lcd != NULL && lcd->fill_rect != NULL, RET_BAD_PARAMS);

  return lcd->fill_rect(lcd, x, y, w, h);
}

ret_t lcd_stroke_rect(lcd_t* lcd, xy_t x, xy_t y, wh_t w, wh_t h) {
  return_value_if_fail(lcd != NULL && lcd->stroke_rect != NULL, RET_BAD_PARAMS);

  return lcd->stroke_rect(lcd, x, y, w, h);
}

ret_t lcd_draw_points(lcd_t* lcd, point_t* points, uint32_t nr) {
  return_value_if_fail(lcd != NULL && lcd->draw_points != NULL && points != NULL, RET_BAD_PARAMS);

  return lcd->draw_points(lcd, points, nr);
}

color_t lcd_get_point_color(lcd_t* lcd, xy_t x, xy_t y) {
  color_t c = color_init(0, 0, 0, 0);
  return_value_if_fail(lcd != NULL && x >= 0 && y >= 0 && x < lcd->w && y < lcd->h, c);

  if (lcd->get_point_color != NULL) {
    return lcd->get_point_color(lcd, x, y);
  } else {
    return lcd->fill_color;
  }
}

ret_t lcd_draw_image(lcd_t* lcd, bitmap_t* img, rect_t* src, rect_t* dst) {
  return_value_if_fail(lcd != NULL && lcd->draw_image != NULL && src != NULL && dst != NULL,
                       RET_BAD_PARAMS);

  return lcd->draw_image(lcd, img, src, dst);
}

ret_t lcd_draw_glyph(lcd_t* lcd, glyph_t* glyph, rect_t* src, xy_t x, xy_t y) {
  return_value_if_fail(lcd != NULL && lcd->draw_glyph != NULL && glyph != NULL && src != NULL,
                       RET_BAD_PARAMS);

  return lcd->draw_glyph(lcd, glyph, src, x, y);
}

wh_t lcd_measure_text(lcd_t* lcd, wchar_t* str, int32_t nr) {
  return_value_if_fail(lcd != NULL && lcd->measure_text != NULL && str != NULL, 0);

  return lcd->measure_text(lcd, str, nr);
}

ret_t lcd_draw_text(lcd_t* lcd, wchar_t* str, int32_t nr, xy_t x, xy_t y) {
  return_value_if_fail(lcd != NULL && lcd->draw_text != NULL && str != NULL, RET_OK);

  return lcd->draw_text(lcd, str, nr, x, y);
}

ret_t lcd_end_frame(lcd_t* lcd) {
  return_value_if_fail(lcd != NULL && lcd->end_frame != NULL, RET_BAD_PARAMS);

  return lcd->end_frame(lcd);
}

ret_t lcd_destroy(lcd_t* lcd) {
  return_value_if_fail(lcd != NULL && lcd->destroy != NULL, RET_BAD_PARAMS);

  return lcd->destroy(lcd);
}

vgcanvas_t* lcd_get_vgcanvas(lcd_t* lcd) {
  return_value_if_fail(lcd != NULL && lcd->get_vgcanvas != NULL, NULL);

  return lcd->get_vgcanvas(lcd);
}

ret_t lcd_take_snapshot(lcd_t* lcd, bitmap_t* img) {
  return_value_if_fail(lcd != NULL && lcd->take_snapshot != NULL, RET_BAD_PARAMS);

  return lcd->take_snapshot(lcd, img);
}
