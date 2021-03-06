/**
 * File:   utils.h
 * Author: Li XianJing <xianjimli@hotmail.com>
 * Brief:  utils function
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

#ifndef UTILS_H
#define UTILS_H

#include "base/types_def.h"

BEGIN_C_DECLS

int unique(wchar_t* str, int size);
char* read_file(const char* file_name, uint32_t* length);
ret_t filename_to_name(const char* filename, char* str, uint32_t size);
ret_t output_c_source(const char* filename, const char* prefix, const char* name, uint8_t* buff,
                      uint32_t size);

ret_t output_res_c_source(const char* filename, uint16_t type, uint16_t subtype, uint8_t* buff,
                          uint32_t size);

const char* to_lower(char* str);
const char* skip_to(const char* p, char c);
const char* skip_char(const char* p, char c);
const char* skip_to_next(const char* p, char c);
bool_t start_with(const char* p, const char* str);
bool_t end_with(const char* p, const char* str);
const char* get_next_token(const char* p, char* token, char c);

END_C_DECLS

#endif  // UTILS_H
