Scriptname ERAS Hidden

; Native API provided by eras.dll

bool Function SetERLevel(int level, bool applyNow = true) global native

bool Function SetAttributes(
  int vig,
  int mnd,
  int end,
  int str,
  int dex,
  int intl,
  int fth,
  int arc,
  bool applyNow = true
) global native

bool Function SetAttributesAndLevel(
  int level,
  int vig,
  int mnd,
  int end,
  int str,
  int dex,
  int intl,
  int fth,
  int arc,
  bool applyNow = true
) global native

bool Function ApplyNow() global native

; attrIndex: 0=vig, 1=mnd, 2=end, 3=str, 4=dex, 5=intl, 6=fth, 7=arc
int Function GetActorERAttr(Actor akActor, int attrIndex) global native

int Function GetActorERLevelForActor(Actor akActor) global native

bool Function IsActorERDerivedFromVanilla(Actor akActor) global native

