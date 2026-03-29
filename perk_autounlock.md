# Perk Auto-Unlock Configuration

Perk auto-unlock is controlled by:

- INI toggle: `EnablePerkAutoUnlock = true`
- JSON file: `SKSE/Plugins/eras_perk_unlocks.json`

## Rule format

Each entry in `autoUnlockPerks` supports:

- `minERLevel` (required): ER level threshold
- `modName` + `formId` (recommended for perk overhauls)
- `editorId` (fallback; useful for vanilla/stable IDs)

### Recommended robust format

```json
{ "modName": "Ordinator - Perks of Skyrim.esp", "formId": "0x00012345", "minERLevel": 10 }
```

`formId` is the local form ID in that plugin.

### Fallback format

```json
{ "editorId": "SteelSmithing", "minERLevel": 2 }
```

## Resolution order

For each rule, plugin resolves in this order:

1. `modName + formId` via `TESDataHandler::LookupForm`
2. `editorId` via `TESForm::LookupByEditorID`

If a perk is not found, a warning is logged and processing continues.

## Notes

- This system does not remove perks when ER level decreases.
- It is safe with perk overhauls when rules are authored with `modName+formId`.