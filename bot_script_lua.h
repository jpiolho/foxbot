#pragma once

enum class BotScriptLuaLoadResult { SUCCESS, FAILED, NOT_PRESENT };

class BotScriptLua {
 public:
   static void TerminateLua();
   static BotScriptLuaLoadResult TryLoad();
   static void OnMessage(const char *msg);
};
