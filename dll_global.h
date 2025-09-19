#pragma once

typedef struct {
   int team_no;
   bool provides_ammo;
   bool provides_armor;
   bool provides_health;
   bool is_flag;
} ENTITY_EXTRA_INFO_S;

ENTITY_EXTRA_INFO_S *GetEntityExtraInfo(int idx);
ENTITY_EXTRA_INFO_S *GetOrAddEntityExtraInfo(int idx);

void DispatchKeyValue_Internal(edict_t *pentKeyvalue, const KeyValueData *pkvd);
