/**
 * JSON shape dispatched as `CustomEvent('updateState', { detail })`.
 * Kept in sync with `BuildStateJSON()` in skse_code/src/Prisma.cpp.
 */
export type AttrKey = "vig" | "mnd" | "end" | "str" | "dex" | "int" | "fth" | "arc";

export type DefenseSheet = {
  physical: number;
  magic: number;
  fire: number;
  lightning: number;
  frost: number;
  poison: number;
};

export type ThresholdSheet = {
  immunity: number;
  robustness: number;
  focus: number;
  vitality: number;
  madness: number;
};

export type EquipLoadSheet = {
  max: number;
  light: number;
  medium: number;
  heavy: number;
};

export type DerivedBlock = {
  maxHP: number;
  maxMP: number;
  maxSP: number;
  carryWeight: number;
  defense: DefenseSheet;
  thresholds: ThresholdSheet;
  equipLoad: EquipLoadSheet;
};

export type StatePayload = {
  ready: boolean;
  levelUpMenuOpen: boolean;
  allocationAllowed: boolean;
  attributes: Record<AttrKey, number>;
  points: { spent: number; level: number; pending: number };
  derived: DerivedBlock;
  derivedPending: DerivedBlock;
  gold: {
    current: number;
    levelUpCost: number;
    nextLevelUpCost: number;
    pendingCost: number;
    canLevelUp: boolean;
    canAllocate: boolean;
    canConfirm: boolean;
  };
  /** Per-attribute pending deltas; C++ always emits all eight keys. */
  pending: { attributes: Record<AttrKey, number> };
};
