Scriptname ERLS Hidden

; Native API provided by eldenrimlevelingsystem.dll

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

