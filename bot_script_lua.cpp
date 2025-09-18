#include "extdll.h"
#include <enginecallback.h>
#include <entity_state.h>

#include "bot.h"

// meta mod includes
#include <dllapi.h>
#include <meta_api.h>

#define LUA_IMPL
#include "minilua.h"
#include "bot_script_lua.h"

#include <string>

#include <unordered_map>

lua_State *lua;

std::unordered_map<std::string, std::vector<int>> msg_handlers;

extern bool blue_av[8];
extern bool red_av[8];
extern bool green_av[8];
extern bool yellow_av[8];

enum LuaTeam { BLUE = 1 << 0, RED = 1 << 1, YELLOW = 1 << 2, GREEN = 1 << 3 };

static int l_OnMsg(lua_State *L) {
   const char *msg = luaL_checkstring(L, 1);
   luaL_checktype(L, 2, LUA_TFUNCTION);

   lua_pushvalue(L, 2);
   int ref = luaL_ref(L, LUA_REGISTRYINDEX);

   msg_handlers[msg].push_back(ref);

   return 0;
}

static int l_SetPoint(lua_State *L) {
   int id = luaL_checkinteger(L, 1);
   int teams = luaL_checkinteger(L, 2);
   bool enabled = lua_toboolean(L, 3);

   if (id < 1 || id > 8) {
      return luaL_error(L, "SetPoint: id must be between 1 and 8 (got %d)", id);
   }

   id -= 1;

   if (teams & BLUE) {
      blue_av[id] = enabled;
   }
   if (teams & RED) {
      red_av[id] = enabled;
   }
   if (teams & GREEN) {
      green_av[id] = enabled;
   }
   if (teams & YELLOW) {
      yellow_av[id] = enabled;
   }

   ALERT(at_console, "Lua SetPoint: id=%d teams=%d (B=%d,R=%d,G=%d,Y=%d) enabled=%s\n", id, teams, (teams & BLUE) ? 1 : 0, (teams & RED) ? 1 : 0, (teams & GREEN) ? 1 : 0, (teams & YELLOW) ? 1 : 0, enabled ? "true" : "false");

   return 0;
}

static int l_IsPoint(lua_State *L) {
   int id = luaL_checkinteger(L, 1);
   int teams = luaL_checkinteger(L, 2);
   bool enabled = lua_toboolean(L, 3);

   if (id < 1 || id > 8) {
      return luaL_error(L, "IsPoint: id must be between 1 and 8 (got %d)", id);
   }

   id -= 1;

   if ((teams & BLUE && blue_av[id] != enabled) || (teams & RED && red_av[id] != enabled) || (teams & GREEN && green_av[id] != enabled) || (teams & YELLOW && yellow_av[id] != enabled)) {
      lua_pushboolean(L, 0);
      return 1;
   }

   lua_pushboolean(L, 1);
   return 1;
}

BotScriptLuaLoadResult BotScriptLua::TryLoad() {
   if (lua != nullptr) {
      lua_close(lua);
      lua = nullptr;
   }

   msg_handlers.clear();

   char mapname[64];
   std::strcpy(mapname, STRING(gpGlobals->mapname));
   std::strcat(mapname, ".lua");

   char filename[256];
   UTIL_BuildFileName(filename, 255, "scripts", mapname);

   // Quick check to see if file exists
   std::FILE *check = std::fopen(filename, "r");
   if (check == nullptr) {
      return BotScriptLuaLoadResult::NOT_PRESENT;
   }
   fclose(check);

   lua = luaL_newstate();
   luaL_openlibs(lua);

   lua_pushcfunction(lua, l_OnMsg);
   lua_setglobal(lua, "OnMsg");

   lua_pushcfunction(lua, l_SetPoint);
   lua_setglobal(lua, "SetPoint");

   lua_pushcfunction(lua, l_IsPoint);
   lua_setglobal(lua, "IsPoint");

   lua_pushinteger(lua, 1 << 0);
   lua_setglobal(lua, "Blue");
   lua_pushinteger(lua, 1 << 1);
   lua_setglobal(lua, "Red");
   lua_pushinteger(lua, 1 << 2);
   lua_setglobal(lua, "Yellow");
   lua_pushinteger(lua, 1 << 3);
   lua_setglobal(lua, "Green");

   lua_pushboolean(lua, 1);
   lua_setglobal(lua, "Available");
   lua_pushboolean(lua, 0);
   lua_setglobal(lua, "NotAvailable");

   int status = luaL_loadfile(lua, filename);
   if (status != LUA_OK) {
      const char *error = lua_tostring(lua, -1);
      ALERT(at_console, "Failed to load lua script file, even though it exists! Error: %s", error);

      lua_pop(lua, 1);
      lua_close(lua);
      lua = nullptr;
      return BotScriptLuaLoadResult::NOT_PRESENT;
   }

   status = lua_pcall(lua, 0, LUA_MULTRET, 0);
   if (status != LUA_OK) {
      const char *error = lua_tostring(lua, -1);
      ALERT(at_console, "Failed to load lua script: %s\n", error);

      lua_pop(lua, 1);
      lua_close(lua);
      lua = nullptr;
      return BotScriptLuaLoadResult::FAILED;
   }

   return BotScriptLuaLoadResult::SUCCESS;
}

void BotScriptLua::OnMessage(const char *msg) {
   if (lua == nullptr)
      return;

   auto it = msg_handlers.find(msg);
   if (it == msg_handlers.end())
      return;

   auto handlers = it->second;
   for (int ref : handlers) {
      lua_rawgeti(lua, LUA_REGISTRYINDEX, ref);
      if (lua_pcall(lua, 0, 0, 0) != LUA_OK) {
         const char *error = lua_tostring(lua, -1);
         ALERT(at_console, "Lua error in handler for '%s': %s\n", msg, error);
         lua_pop(lua, 1);
      }
   }
}
