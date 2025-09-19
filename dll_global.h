#pragma once

typedef struct {
   int team_no;
} ENTITY_EXTRA_INFO_S;

ENTITY_EXTRA_INFO_S *GetEntityExtraInfo(int idx);
ENTITY_EXTRA_INFO_S *GetOrAddEntityExtraInfo(int idx);
