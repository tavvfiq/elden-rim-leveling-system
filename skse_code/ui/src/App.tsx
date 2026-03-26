import React, { useEffect, useMemo, useState } from "react";

type AttrKey = "vig" | "mnd" | "end" | "str" | "dex" | "int" | "fth" | "arc";

type DefenseSheet = {
  physical: number;
  magic: number;
  fire: number;
  lightning: number;
  frost: number;
  poison: number;
};

type ThresholdSheet = {
  immunity: number;
  robustness: number;
  focus: number;
  vitality: number;
  madness: number;
};

type EquipLoadSheet = {
  max: number;
  light: number;
  medium: number;
  heavy: number;
};

type DerivedBlock = {
  maxHP: number;
  maxMP: number;
  maxSP: number;
  carryWeight: number;
  defense?: DefenseSheet;
  thresholds?: ThresholdSheet;
  equipLoad?: EquipLoadSheet;
};

type StatePayload = {
  ready: boolean;
  levelUpMenuOpen: boolean;
  attributes: Record<AttrKey, number>;
  points: { spent: number; level: number; unspent: number };
  derived: DerivedBlock;
  derivedPending: DerivedBlock;
  gold: { current: number; levelUpCost: number; canLevelUp: boolean };
  pending: { attributes: Partial<Record<AttrKey, number>> };
};

declare global {
  interface Window {
    requestInitState?: (args: string) => void;
    levelUp?: (args: string) => void;
    allocatePoint?: (args: string) => void;
    refundPoint?: (args: string) => void;
    confirmAllocation?: (args: string) => void;
    cancelAllocation?: (args: string) => void;
  }
}

const statLabels: Record<AttrKey, string> = {
  vig: "Vigor",
  mnd: "Mind",
  end: "Endurance",
  str: "Strength",
  dex: "Dexterity",
  int: "Intelligence",
  fth: "Faith",
  arc: "Arcane",
};

function fmtSheet(n: number | undefined): string {
  if (n === undefined || Number.isNaN(n)) return "—";
  return String(Math.round(n));
}

export function App() {
  const [state, setState] = useState<StatePayload | null>(null);

  useEffect(() => {
    const onUpdate = (ev: Event) => {
      const ce = ev as CustomEvent;
      setState(ce.detail as StatePayload);
    };
    window.addEventListener("updateState", onUpdate as any);

    if (typeof window.requestInitState === "function") {
      window.requestInitState("");
    }

    return () => window.removeEventListener("updateState", onUpdate as any);
  }, []);

  const rows = useMemo(() => {
    const attrs = state?.attributes;
    if (!attrs) return [];
    return (Object.keys(attrs) as AttrKey[]).map((k) => ({ key: k, value: attrs[k] }));
  }, [state]);

  const pending: Partial<Record<AttrKey, number>> = state?.pending.attributes ?? {};
  const unspent = state?.points.unspent ?? 0;
  const hasPending = Object.values(pending).some((v) => (v ?? 0) > 0);

  const d = state?.derived;
  const dp = state?.derivedPending;

  const send = (fn: keyof Window, payload: unknown) => {
    const f = (window as any)[fn];
    if (typeof f === "function") f(JSON.stringify(payload ?? {}));
  };

  return (
    <div className="wrap">
      <div className="panelFrame">
        <span className="cornerBotLeft" aria-hidden="true"></span>
        <span className="cornerBotRight" aria-hidden="true"></span>

        <div className="topbar">
          <div>
            <div className="title">Level Up</div>
          </div>
        </div>

        <div className="layout">
          <div className="left">
            <div className="section">
              <div className="kv">
                <div className="kvRow">
                  <div className="label">Level</div>
                  <div className="val">{state?.points.level ?? 1}</div>
                  <div className="arrow">→</div>
                  <div className="val">{(state?.points.level ?? 1) + 1}</div>
                </div>
                <div className="kvRow">
                  <div className="label">Gold Held</div>
                  <div className="val">{state?.gold.current ?? 0}</div>
                  <div className="arrow">→</div>
                  <div className="val">
                    {(state?.gold.current ?? 0) - (state?.gold.levelUpCost ?? 0)}
                  </div>
                </div>
                <div className="kvRow">
                  <div className="label">Gold Needed</div>
                  <div className="val"></div>
                  <div className="arrow"></div>
                  <div className="val danger">{state?.gold.levelUpCost ?? 0}</div>
                </div>
              </div>

              {state?.gold.canLevelUp ? (
                <div className="levelupWrap">
                  <button className="primaryBtn" onClick={() => send("levelUp", {})}>
                    Level Up
                  </button>
                </div>
              ) : (
                <div className="levelupWrap">
                  <div className="levelupNote">Not enough gold to level up.</div>
                </div>
              )}
            </div>

            <div className="section">
              <div className="sectionTitle">Attribute Points</div>
              <div className="subtitle" style={{ marginBottom: 8 }}>
                Unspent: {unspent}
              </div>

              <div className="kv">
                {rows.map((r: { key: AttrKey; value: number }) => (
                  <div className="kvRow" key={r.key}>
                    <div className="label">{statLabels[r.key]}</div>
                    <div className="val">{r.value}</div>
                    <div className="arrow">→</div>
                    <div className="val">
                      <span className="spin">
                        <button
                          onClick={() => send("refundPoint", { attr: r.key })}
                          disabled={!pending[r.key]}
                        >
                          ‹
                        </button>
                        {r.value + (pending[r.key] ?? 0)}
                        <button
                          onClick={() => send("allocatePoint", { attr: r.key })}
                          disabled={unspent <= 0}
                        >
                          ›
                        </button>
                      </span>
                    </div>
                  </div>
                ))}
              </div>

              <div style={{ marginTop: 10 }}>
                <button
                  className="primaryBtn"
                  onClick={() => send("confirmAllocation", {})}
                  disabled={!hasPending}
                >
                  Confirm
                </button>
                <button className="ghostBtn" onClick={() => send("cancelAllocation", {})}>
                  Cancel
                </button>
              </div>
            </div>
          </div>

          <div className="right">
            <div className="section">
              <div className="sectionTitle">Base Stats</div>
              <div className="kv">
                <div className="kvRow">
                  <div className="label">HP</div>
                  <div className="val">{state?.derived.maxHP ?? 0}</div>
                  <div className="arrow">→</div>
                  <div className="val">{state?.derivedPending.maxHP ?? 0}</div>
                </div>
                <div className="kvRow">
                  <div className="label">MP</div>
                  <div className="val">{state?.derived.maxMP ?? 0}</div>
                  <div className="arrow">→</div>
                  <div className="val">{state?.derivedPending.maxMP ?? 0}</div>
                </div>
                <div className="kvRow">
                  <div className="label">Stamina</div>
                  <div className="val">{state?.derived.maxSP ?? 0}</div>
                  <div className="arrow">→</div>
                  <div className="val">{state?.derivedPending.maxSP ?? 0}</div>
                </div>
                <div className="kvRow">
                  <div className="label">Carry Weight</div>
                  <div className="val">{state?.derived.carryWeight ?? 0}</div>
                  <div className="arrow">→</div>
                  <div className="val">{state?.derivedPending.carryWeight ?? 0}</div>
                </div>
                <div className="kvRow">
                  <div className="label">Equip load max</div>
                  <div className="val">{fmtSheet(d?.equipLoad?.max)}</div>
                  <div className="arrow">→</div>
                  <div className="val">{fmtSheet(dp?.equipLoad?.max)}</div>
                </div>
                <div className="kvRow">
                  <div className="label">Equip light tier</div>
                  <div className="val">{fmtSheet(d?.equipLoad?.light)}</div>
                  <div className="arrow">→</div>
                  <div className="val">{fmtSheet(dp?.equipLoad?.light)}</div>
                </div>
                <div className="kvRow">
                  <div className="label">Equip medium tier</div>
                  <div className="val">{fmtSheet(d?.equipLoad?.medium)}</div>
                  <div className="arrow">→</div>
                  <div className="val">{fmtSheet(dp?.equipLoad?.medium)}</div>
                </div>
              </div>
            </div>

            <div className="section">
              <div className="sectionTitle">Defense Power</div>
              <div className="kv">
                <div className="kvRow">
                  <div className="label">Physical</div>
                  <div className="val">{fmtSheet(d?.defense?.physical)}</div>
                  <div className="arrow">→</div>
                  <div className="val">{fmtSheet(dp?.defense?.physical)}</div>
                </div>
                <div className="kvRow">
                  <div className="label">Magic</div>
                  <div className="val">{fmtSheet(d?.defense?.magic)}</div>
                  <div className="arrow">→</div>
                  <div className="val">{fmtSheet(dp?.defense?.magic)}</div>
                </div>
                <div className="kvRow">
                  <div className="label">Fire</div>
                  <div className="val">{fmtSheet(d?.defense?.fire)}</div>
                  <div className="arrow">→</div>
                  <div className="val">{fmtSheet(dp?.defense?.fire)}</div>
                </div>
                <div className="kvRow">
                  <div className="label">Lightning</div>
                  <div className="val">{fmtSheet(d?.defense?.lightning)}</div>
                  <div className="arrow">→</div>
                  <div className="val">{fmtSheet(dp?.defense?.lightning)}</div>
                </div>
                <div className="kvRow">
                  <div className="label">Frost</div>
                  <div className="val">{fmtSheet(d?.defense?.frost)}</div>
                  <div className="arrow">→</div>
                  <div className="val">{fmtSheet(dp?.defense?.frost)}</div>
                </div>
                <div className="kvRow">
                  <div className="label">Poison</div>
                  <div className="val">{fmtSheet(d?.defense?.poison)}</div>
                  <div className="arrow">→</div>
                  <div className="val">{fmtSheet(dp?.defense?.poison)}</div>
                </div>
              </div>
            </div>

            <div className="section" style={{ gridColumn: "2 / 3" }}>
              <div className="sectionTitle">Body (thresholds)</div>
              <div className="kv">
                <div className="kvRow">
                  <div className="label">Immunity</div>
                  <div className="val">{fmtSheet(d?.thresholds?.immunity)}</div>
                  <div className="arrow">→</div>
                  <div className="val">{fmtSheet(dp?.thresholds?.immunity)}</div>
                </div>
                <div className="kvRow">
                  <div className="label">Robustness</div>
                  <div className="val">{fmtSheet(d?.thresholds?.robustness)}</div>
                  <div className="arrow">→</div>
                  <div className="val">{fmtSheet(dp?.thresholds?.robustness)}</div>
                </div>
                <div className="kvRow">
                  <div className="label">Focus</div>
                  <div className="val">{fmtSheet(d?.thresholds?.focus)}</div>
                  <div className="arrow">→</div>
                  <div className="val">{fmtSheet(dp?.thresholds?.focus)}</div>
                </div>
                <div className="kvRow">
                  <div className="label">Vitality</div>
                  <div className="val">{fmtSheet(d?.thresholds?.vitality)}</div>
                  <div className="arrow">→</div>
                  <div className="val">{fmtSheet(dp?.thresholds?.vitality)}</div>
                </div>
                <div className="kvRow">
                  <div className="label">Madness</div>
                  <div className="val">{fmtSheet(d?.thresholds?.madness)}</div>
                  <div className="arrow">→</div>
                  <div className="val">{fmtSheet(dp?.thresholds?.madness)}</div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

