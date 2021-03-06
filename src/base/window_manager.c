/**
 * File:   window_manager.c
 * Author: Li XianJing <xianjimli@hotmail.com>
 * Brief:  window manager
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

#include "base/keys.h"
#include "base/mem.h"
#include "base/idle.h"
#include "base/timer.h"
#include "base/platform.h"
#include "base/prop_names.h"
#include "base/window_manager.h"

static widget_t* window_manager_find_prev_window(widget_t* widget) {
  int32_t i = 0;
  int32_t nr = 0;
  return_value_if_fail(widget != NULL, NULL);

  if (widget->children != NULL && widget->children->size > 0) {
    nr = widget->children->size;
    for (i = nr - 2; i >= 0; i--) {
      widget_t* iter = (widget_t*)(widget->children->elms[i]);
      if (iter->type == WIDGET_NORMAL_WINDOW) {
        return iter;
      }
    }
  }

  return NULL;
}

static ret_t window_manager_check_if_need_open_animation(const idle_info_t* info) {
  value_t anim_hint;
  widget_t* prev_win = NULL;
  widget_t* curr_win = WIDGETP(info->ctx);
  window_manager_t* wm = WINDOW_MANAGER(curr_win->parent);

  if (wm->animator != NULL) {
    return RET_OK;
  }

  prev_win = window_manager_find_prev_window((widget_t*)wm);
  return_value_if_fail(prev_win != NULL, RET_FAIL);

  if (widget_get_prop(curr_win, WIDGET_PROP_ANIM_HINT, &anim_hint) == RET_OK) {
    uint32_t type = value_int(&anim_hint);
    if (type > WINDOW_ANIMATOR_NONE && type < WINDOW_ANIMATOR_NR) {
      wm->animator = window_animator_create_for_open(type, wm->canvas, prev_win, curr_win);
      wm->animating = wm->animator != NULL;
      if(wm->animating) {
        wm->ignore_user_input = TRUE;
        log_debug("ignore_user_input\n");
      }
    }
  }

  return RET_OK;
}

static ret_t window_manager_check_if_need_close_animation(window_manager_t* wm,
                                                          widget_t* curr_win) {
  value_t anim_hint;
  widget_t* prev_win = NULL;

  if (wm->animator != NULL) {
    return RET_FAIL;
  }

  prev_win = window_manager_find_prev_window((widget_t*)wm);
  return_value_if_fail(prev_win != NULL, RET_FAIL);

  if (widget_get_prop(curr_win, WIDGET_PROP_ANIM_HINT, &anim_hint) == RET_OK) {
    uint32_t type = value_int(&anim_hint);
    if (type > WINDOW_ANIMATOR_NONE && type < WINDOW_ANIMATOR_NR) {
      wm->animator = window_animator_create_for_close(type, wm->canvas, prev_win, curr_win);
      wm->animating = wm->animator != NULL;
      if(wm->animating) {
        wm->ignore_user_input = TRUE;
        log_debug("ignore_user_input\n");
      }
      return wm->animator != NULL ? RET_OK : RET_FAIL;
    }
  }

  return RET_FAIL;
}

static ret_t window_manager_remove_child_real(widget_t* wm, widget_t* window) {
  ret_t ret = RET_OK;
  return_value_if_fail(wm != NULL && window != NULL, RET_BAD_PARAMS);

  ret = widget_remove_child(wm, window);
  if (ret == RET_OK) {
    widget_t* top = window_manager_get_top_window(wm);
    if (top) {
      rect_t r;
      rect_init(r, window->x, window->y, window->w, window->h);
      widget_invalidate(top, &r);
    }
  }

  return ret;
}

static ret_t on_window_destroy(void* ctx, event_t* e) {
  widget_t* wm = WIDGETP(ctx);
  if (array_find(wm->children, NULL, e->target)) {
    window_manager_remove_child_real(wm, e->target);
  }

  return RET_OK;
}

ret_t window_manager_add_child(widget_t* wm, widget_t* window) {
  ret_t ret = RET_OK;
  return_value_if_fail(wm != NULL && window != NULL, RET_BAD_PARAMS);

  if (window->type == WIDGET_NORMAL_WINDOW) {
    widget_move_resize(window, 0, 0, wm->w, wm->h);
  } else if (window->type == WIDGET_DIALOG) {
    xy_t x = (wm->w - window->w) >> 1;
    xy_t y = (wm->h - window->h) >> 1;
    widget_move_resize(window, x, y, window->w, window->h);
  }

  widget_invalidate(window, NULL);
  widget_on(window, EVT_DESTROY, on_window_destroy, wm);

  if (wm->children != NULL && wm->children->size > 0) {
    idle_add((idle_func_t)window_manager_check_if_need_open_animation, window);
  }

  ret = widget_add_child(wm, window);
  if(ret == RET_OK) {
    wm->target = window;
  }

  return ret;
}

static ret_t window_manager_idle_destroy_window(const idle_info_t* info) {
  widget_t* win = WIDGETP(info->ctx);
  widget_destroy(win);

  return RET_OK;
}

ret_t window_manager_remove_child(widget_t* wm, widget_t* window) {
  ret_t ret = RET_OK;
  return_value_if_fail(wm != NULL && window != NULL, RET_BAD_PARAMS);

  if (window_manager_check_if_need_close_animation(WINDOW_MANAGER(wm), window) != RET_OK) {
    window_manager_remove_child_real(wm, window);
    idle_add(window_manager_idle_destroy_window, window);
  }

  return ret;
}

widget_t* window_manager_find_target(widget_t* widget, xy_t x, xy_t y) {
  uint32_t i = 0;
  uint32_t n = 0;
  point_t p = {x, y};
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(widget != NULL, NULL);

  if (wm->graps.size > 0) {
    return (widget_t*)(wm->graps.elms[wm->graps.size - 1]);
  }

  widget_to_local(widget, &p);
  if (widget->children != NULL && widget->children->size > 0) {
    xy_t xx = p.x;
    xy_t yy = p.y;
    n = widget->children->size;
    for (i = n; i > 0; i--) {
      widget_t* iter = (widget_t*)(widget->children->elms[i - 1]);
      xy_t r = iter->x + iter->w;
      xy_t b = iter->y + iter->h;

      if (xx >= iter->x && yy >= iter->y && xx <= r && yy <= b) {
        return iter;
      }

      if (iter->type == WIDGET_NORMAL_WINDOW || iter->type == WIDGET_DIALOG) {
        return iter;
      }
    }
  }

  return NULL;
}

static ret_t window_manager_paint_normal(widget_t* widget, canvas_t* c) {
  rect_t r;
  rect_t* dr = NULL;
  rect_t* ldr = NULL;
  window_manager_t* wm = WINDOW_MANAGER(widget);

  dr = &(wm->dirty_rect);

  if (dr->w && dr->h) {
    ldr = &(wm->last_dirty_rect);

    r = *dr;
    rect_merge(&r, ldr);

    if (r.w > 0 && r.h > 0) {
      ENSURE(canvas_begin_frame(c, &r, LCD_DRAW_NORMAL) == RET_OK);
      ENSURE(widget_paint(WIDGETP(wm), c) == RET_OK);
      ENSURE(canvas_end_frame(c) == RET_OK);
    }
    log_debug("%s x=%d y=%d w=%d h=%d\n", __func__, r.x, r.y, r.w, r.h);
  }

  wm->last_dirty_rect = wm->dirty_rect;
  rectp_init(dr, widget->w, widget->h, 0, 0);

  return RET_OK;
}

static ret_t timer_enable_user_input(const timer_info_t* timer) {
  window_manager_t* wm = WINDOW_MANAGER(timer->ctx);
 
  wm->ignore_user_input = FALSE;
  log_debug("enable user input\n");

  return RET_OK;
}

static ret_t window_manager_paint_animation(widget_t* widget, canvas_t* c) {
  uint32_t time_ms = get_time_ms();
  window_manager_t* wm = WINDOW_MANAGER(widget);

  ret_t ret = window_animator_update(wm->animator, time_ms);
  if (ret == RET_DONE) {
    window_animator_destroy(wm->animator);
    wm->animator = NULL;
    wm->animating = FALSE;
    timer_add(timer_enable_user_input, wm, 500);
  }

  return RET_OK;
}

ret_t window_manager_paint(widget_t* widget, canvas_t* c) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(wm != NULL && c != NULL, RET_BAD_PARAMS);

  wm->canvas = c;
  if (wm->animator != NULL) {
    return window_manager_paint_animation(widget, c);
  } else {
    return window_manager_paint_normal(widget, c);
  }
}

widget_t* default_wm() {
  static window_manager_t* wm = NULL;
  if (wm == NULL) {
    wm = MEM_ZALLOC(window_manager_t);
    window_manager_init(wm);
  }

  return WIDGETP(wm);
}

static ret_t window_manager_grab(widget_t* widget, widget_t* child) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(widget != NULL && child != NULL, RET_BAD_PARAMS);
  array_push(&(wm->graps), child);

  return RET_OK;
}

static ret_t window_manager_ungrab(widget_t* widget, widget_t* child) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(widget != NULL && child != NULL, RET_BAD_PARAMS);
  array_remove(&(wm->graps), NULL, child);

  return RET_OK;
}

static ret_t window_manager_invalidate(widget_t* widget, rect_t* r) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  rect_t* dr = &(wm->dirty_rect);

  rect_merge(dr, r);

  return RET_OK;
}

int32_t window_manager_find_top_window_index(widget_t* widget) {
  int32_t i = 0;
  int32_t nr = 0;
  return_value_if_fail(widget != NULL, -1);

  if (widget->children != NULL && widget->children->size > 0) {
    nr = widget->children->size;
    for (i = nr - 1; i >= 0; i--) {
      widget_t* iter = (widget_t*)(widget->children->elms[i]);
      if (iter->type == WIDGET_NORMAL_WINDOW) {
        return i;
      }
    }
  }

  return -1;
}

widget_t* window_manager_get_top_window(widget_t* widget) {
  int32_t index = window_manager_find_top_window_index(widget);

  return widget_get_child(widget, index);
}

ret_t window_manager_on_paint_children(widget_t* widget, canvas_t* c) {
  int32_t i = 0;
  int32_t nr = 0;
  return_value_if_fail(widget != NULL && c != NULL, RET_BAD_PARAMS);

  if (widget->children != NULL && widget->children->size > 0) {
    nr = widget->children->size;

    for (i = nr - 1; i >= 0; i--) {
      widget_t* iter = (widget_t*)(widget->children->elms[i]);
      if (iter->type == WIDGET_NORMAL_WINDOW) {
        break;
      }
    }

    if (i < 0) {
      i = 0;
    }

    for (; i < nr; i++) {
      widget_t* iter = (widget_t*)(widget->children->elms[i]);
      if (iter->visible) {
        widget_paint(iter, c);
      }
    }
  }

  return RET_OK;
}

static const widget_vtable_t s_wm_vtable = {.invalidate = window_manager_invalidate,
                                            .on_paint_children = window_manager_on_paint_children,
                                            .grab = window_manager_grab,
                                            .find_target = window_manager_find_target,
                                            .ungrab = window_manager_ungrab};

widget_t* window_manager_init(window_manager_t* wm) {
  widget_t* w = &(wm->widget);
  return_value_if_fail(wm != NULL, NULL);

  widget_init(w, NULL, WIDGET_WINDOW_MANAGER);
  array_init(&(wm->graps), 5);
  w->vt = &s_wm_vtable;

  return w;
}

ret_t window_manager_resize(widget_t* widget, wh_t w, wh_t h) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(wm != NULL, RET_BAD_PARAMS);

  wm->dirty_rect.x = 0;
  wm->dirty_rect.y = 0;
  wm->dirty_rect.w = w;
  wm->dirty_rect.h = h;
  wm->last_dirty_rect = wm->dirty_rect;
  widget_move_resize(widget, 0, 0, w, h);

  return RET_OK;
}

static ret_t window_manager_update_key_status(window_manager_t* wm, uint32_t key, bool_t down) {
  if (key == FKEY_LSHIFT || key == FKEY_RSHIFT) {
    wm->shift = down;
  }
  if (key == FKEY_LALT || key == FKEY_RALT) {
    wm->alt = down;
  }
  if (key == FKEY_LCTRL || key == FKEY_RCTRL) {
    wm->ctrl = down;
  }
  if (key == FKEY_CAPSLOCK) {
    wm->caplock = down;
  }

  return RET_OK;
}

typedef struct _key_shift_t {
  char key;
  char shift_key;
} key_shift_t;

static const key_shift_t key_shift[] = {
    {'`', '~'}, {'1', '!'}, {'2', '@'}, {'3', '#'},  {'4', '$'}, {'5', '%'}, {'6', '^'},
    {'7', '&'}, {'8', '*'}, {'9', '('}, {'0', ')'},  {'-', '_'}, {'=', '+'}, {'[', '{'},
    {']', '}'}, {',', '<'}, {'.', '>'}, {'\\', '|'}, {'/', '?'},
};

static ret_t window_manager_shift_key(window_manager_t* wm, key_event_t* e) {
  char c = (char)e->key;
  if (wm->shift) {
    uint32_t i = 0;
    for (i = 0; i < ARRAY_SIZE(key_shift); i++) {
      if (key_shift[i].key == c) {
        e->key = key_shift[i].shift_key;
        return RET_OK;
      }
    }
  }

  if (wm->shift && wm->caplock) {
    return RET_OK;
  }

  if (wm->shift || wm->caplock) {
    if (c >= FKEY_a && c <= FKEY_z) {
      e->key = c - 32;
    }
  }

  return RET_OK;
}

ret_t window_manager_dispatch_input_event(widget_t* widget, event_t* e) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(wm != NULL && e != NULL, RET_BAD_PARAMS);

  if (wm->ignore_user_input) {
    log_debug("animating ignore input\n");
    return RET_OK;
  }

  switch (e->type) {
    case EVT_POINTER_DOWN: {
      pointer_event_t* evt = (pointer_event_t*)e;
      evt->alt = wm->alt;
      evt->ctrl = wm->ctrl;
      evt->shift = wm->shift;
      widget_on_pointer_down(widget, evt);
      break;
    }
    case EVT_POINTER_MOVE: {
      pointer_event_t* evt = (pointer_event_t*)e;
      evt->alt = wm->alt;
      evt->ctrl = wm->ctrl;
      evt->shift = wm->shift;
      widget_on_pointer_move(widget, evt);
      break;
    }
    case EVT_POINTER_UP: {
      pointer_event_t* evt = (pointer_event_t*)e;
      evt->alt = wm->alt;
      evt->ctrl = wm->ctrl;
      evt->shift = wm->shift;
      widget_on_pointer_up(widget, evt);
      break;
    }
    case EVT_KEY_DOWN: {
      key_event_t* evt = (key_event_t*)e;
      window_manager_update_key_status(wm, evt->key, TRUE);
      evt->alt = wm->alt;
      evt->ctrl = wm->ctrl;
      evt->shift = wm->shift;
      evt->caplock = wm->caplock;

      window_manager_shift_key(wm, evt);
      widget_on_keydown(widget, evt);
      break;
    }
    case EVT_KEY_UP: {
      key_event_t* evt = (key_event_t*)e;

      evt->alt = wm->alt;
      evt->ctrl = wm->ctrl;
      evt->shift = wm->shift;
      evt->caplock = wm->caplock;

      window_manager_shift_key(wm, evt);
      widget_on_keyup(widget, evt);

      window_manager_update_key_status(wm, evt->key, FALSE);
      break;
    }
  }

  return RET_OK;
}

ret_t window_manager_set_animating(widget_t* widget, bool_t animating) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(wm != NULL, RET_BAD_PARAMS);

  wm->animating = animating;

  return RET_OK;
}
