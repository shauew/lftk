/**
 * File:   font_bitmap.h
 * Author: Li XianJing <xianjimli@hotmail.com>
 * Brief:  font bitmap font loader
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
 * 2018-01-21 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef LFTK_FONT_BITMAP_H
#define LFTK_FONT_BITMAP_H

#include "base/font.h"

BEGIN_C_DECLS

typedef struct _font_bitmap_index_t {
  uint16_t c;
  uint16_t offset;
} font_bitmap_index_t;

typedef struct _font_bitmap_header_t {
  uint16_t char_nr;
  uint16_t font_size;
  font_bitmap_index_t index[1];
} font_bitmap_header_t;

font_t* font_bitmap_create(const char* name, const uint8_t* buff, uint32_t buff_size);

END_C_DECLS

#endif /*LFTK_FONT_BITMAP_H*/
