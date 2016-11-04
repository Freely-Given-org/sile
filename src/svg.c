#include <stdio.h>
#include <string.h>
#include <math.h>
#define NANOSVG_IMPLEMENTATION  // Expands implementation
#include "nanosvg.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

int svg_to_ps(lua_State *L) {
  const char* input = luaL_checkstring(L, 1);
  struct NSVGimage* image;
  image = nsvgParse(input, "pt", 72);
  int max_output = 256;
  int output_l = 0;
  char *output = malloc(max_output);
  output[0] = '\0';
  for (NSVGshape *shape = image->shapes; shape != NULL; shape = shape->next) {
      for (NSVGpath *path = shape->paths; path != NULL; path = path->next) {
          for (int i = 0; i < path->npts-1; i += 3) {
              float* p = &path->pts[i*2];
              char thisPath[256];
              // Some kind of precision here?
              snprintf(thisPath, 256, "%f %f m %f %f %f %f %f %f c ",
                p[0],p[1], p[2],p[3], p[4],p[5], p[6],p[7]);
              if (output_l + strlen(thisPath) > max_output) {
                max_output *= 2;
                output = realloc(output, max_output);
              }
              strncat(output, thisPath, max_output);
              output_l += strlen(thisPath);
          }
      }
      char strokeFillOper = 'S';
      if (shape->fill.type == NSVG_PAINT_COLOR) {
        strokeFillOper = 'F';
        if (shape->stroke.type == NSVG_PAINT_COLOR) {
          strokeFillOper = 'B';
        }
      }
      if (output_l + 3 > max_output) { // How unlucky
        output = realloc(output, max_output + 3);
      }
      output[output_l++] = strokeFillOper;
      output[output_l++] = ' ';
      output[output_l] = '\0';
  }
  lua_pushstring(L, output);
  lua_pushnumber(L, image->width);
  lua_pushnumber(L, image->height);
  free(output);
  // Delete
  nsvgDelete(image);
  return 3;
}

#if !defined LUA_VERSION_NUM
/* Lua 5.0 */
#define luaL_Reg luaL_reg
#endif

#if !defined LUA_VERSION_NUM || LUA_VERSION_NUM==501
/*
** Adapted from Lua 5.2.0
*/
static void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup) {
  luaL_checkstack(L, nup+1, "too many upvalues");
  for (; l->name != NULL; l++) {  /* fill the table with given functions */
    int i;
    lua_pushstring(L, l->name);
    for (i = 0; i < nup; i++)  /* copy upvalues to the top */
      lua_pushvalue(L, -(nup+1));
    lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    lua_settable(L, -(nup + 3));
  }
  lua_pop(L, nup);  /* remove upvalues */
}
#endif

static const struct luaL_Reg lib_table [] = {
  {"svg_to_ps", svg_to_ps},
  {NULL, NULL}
};

int luaopen_svg (lua_State *L) {
  lua_newtable(L);
  luaL_setfuncs(L, lib_table, 0);
  return 1;
}