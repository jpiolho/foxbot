#pragma once

enum class BotScriptLuaLoadResult { SUCCESS, FAILED, NOT_PRESENT };

class BotScriptLua {
 public:
   static BotScriptLuaLoadResult TryLoad();
   static void OnMessage(const char *msg);
};
