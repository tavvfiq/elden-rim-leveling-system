import React, { useEffect, useMemo, useState } from "react";
import { MOCK_LEVEL_UP_STATE } from "./mockLevelUpState";
import type { AttrKey, StatePayload } from "./stateTypes";

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

function trendClass(current: number | undefined, next: number | undefined): string {
  if (
    current === undefined ||
    next === undefined ||
    Number.isNaN(current) ||
    Number.isNaN(next)
  ) {
    return "";
  }
  if (next > current) return " up";
  if (next < current) return " down";
  return "";
}

function useMockPreviewEnabled(): boolean {
  return (
    typeof window !== "undefined" &&
    (import.meta.env.DEV || new URLSearchParams(window.location.search).has("mock"))
  );
}

export function App() {
  const mockPreview = useMockPreviewEnabled();
  const [state, setState] = useState<StatePayload | null>(mockPreview ? MOCK_LEVEL_UP_STATE : null);

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

  const send = (fn: keyof Window, payload: unknown) => {
    const f = (window as any)[fn];
    if (typeof f === "function") f(JSON.stringify(payload ?? {}));
  };

  if (!state?.ready) {
    return (
      <div className="erRoot">
        <div className="erBackdrop" aria-hidden="true" />
        <div className="erLoading">…</div>
      </div>
    );
  }

  const pending = state.pending.attributes;
  const pendingPoints = state.points.pending;
  const hasPending = pendingPoints > 0;
  const goldCurrent = state.gold.current;
  const goldNeeded = state.gold.nextLevelUpCost;
  const hasEnoughGold = goldCurrent - state.gold.pendingCost >= goldNeeded;

  const d = state.derived;
  const dp = state.derivedPending;

  const viewOnly = !state.allocationAllowed;

  const decreaseButton = "<";
  const increaseButton = ">";

  return (
    <div className="erRoot">
      <div className="erBackdrop" aria-hidden="true" />
      <div className="erVignette" aria-hidden="true" />

      <div className="erShell">
        <header className="erHeader">
          <div className="erHeaderTitleRow">
            <span className="erIcon" aria-hidden="true" />
            <div>
              <h1 className="erTitle">Level Up</h1>
              {viewOnly ? (
                <p className="erSubtitle">View only — rest in a bed to level attributes.</p>
              ) : (
                <p className="erSubtitleMuted">Choose attribute to level up.</p>
              )}
            </div>
          </div>
        </header>

        <main className="erMain">
          <section className="erCol erColAttr">
            <div className="erHeaderRunes erHeaderRunesInCol" aria-label="Level and gold">
              <div className="erRuneRow">
                <span className="erRuneLabel">Level</span>
                <span className="erRuneVal">{state.points.level}</span>
                <span className="erRuneArrow">→</span>
                <span className={`erRuneNext${trendClass(state.points.level, state.points.level + pendingPoints)}`}>
                  {state.points.level + pendingPoints}
                </span>
              </div>
              <div className="erRuneRow">
                <span className="erRuneLabel">Gold held</span>
                <span className="erRuneVal">{goldCurrent}</span>
                <span className="erRuneArrow">→</span>
                <span className={`erRuneNext${hasEnoughGold ? "" : " erDanger"}`}>
                  {goldCurrent - state.gold.pendingCost}
                </span>
              </div>
              <div className="erRuneRow">
                <span className="erRuneLabel">Gold needed</span>
                <span className="erRuneVal" />
                <span className="erRuneArrow"></span>
                <span className={`erRuneNext${hasEnoughGold ? "" : " erDanger"}`}>{state.gold.nextLevelUpCost}</span>
              </div>
            </div>
            <h2 className="erColTitle">Attributes</h2>
            <div className="erKv">
              {rows.map((r) => (
                <div className="erKvRow" key={r.key}>
                  <span className="erLabel">{statLabels[r.key]}</span>
                  <span className="erVal erValCur">{r.value}</span>
                  <span className="erArrow">→</span>
                  <span className={`erVal erValNext${trendClass(r.value, r.value + (pending[r.key] ?? 0))}`}>
                    <span className="erSpin">
                      <button
                        type="button"
                        className="erSpinBtn"
                        onClick={() => send("refundPoint", { attr: r.key })}
                        disabled={viewOnly || !pending[r.key]}
                        aria-label="Decrease"
                      >
                        {decreaseButton}
                      </button>
                      <span className="erSpinNum">{r.value + (pending[r.key] ?? 0)}</span>
                      <button
                        type="button"
                        className="erSpinBtn"
                        onClick={() => send("allocatePoint", { attr: r.key })}
                        disabled={viewOnly || !state.gold.canAllocate}
                        aria-label="Increase"
                      >
                        {increaseButton}
                      </button>
                    </span>
                  </span>
                </div>
              ))}
            </div>
            <div className="erActions">
              <button
                type="button"
                className="erBtnPrimary"
                onClick={() => send("confirmAllocation", {})}
                disabled={viewOnly || !hasPending || !state.gold.canConfirm}
              >
                Confirm
              </button>
              <button type="button" className="erBtnGhost" onClick={() => send("cancelAllocation", {})}>
                Cancel
              </button>
            </div>
          </section>

          <section className="erCol erColMid">
            <h2 className="erColTitle">Base stats</h2>
            <div className="erKv">
              <div className="erKvRow">
                <span className="erLabel">HP</span>
                <span className="erVal erValCur">{state.derived.maxHP}</span>
                <span className="erArrow">→</span>
                <span className={`erVal erValNext${trendClass(state.derived.maxHP, state.derivedPending.maxHP)}`}>
                  {state.derivedPending.maxHP}
                </span>
              </div>
              <div className="erKvRow">
                <span className="erLabel">MP</span>
                <span className="erVal erValCur">{state.derived.maxMP}</span>
                <span className="erArrow">→</span>
                <span className={`erVal erValNext${trendClass(state.derived.maxMP, state.derivedPending.maxMP)}`}>
                  {state.derivedPending.maxMP}
                </span>
              </div>
              <div className="erKvRow">
                <span className="erLabel">Stamina</span>
                <span className="erVal erValCur">{state.derived.maxSP}</span>
                <span className="erArrow">→</span>
                <span className={`erVal erValNext${trendClass(state.derived.maxSP, state.derivedPending.maxSP)}`}>
                  {state.derivedPending.maxSP}
                </span>
              </div>
              <div className="erKvRow">
                <span className="erLabel">Max equip load</span>
                <span className="erVal erValCur">{fmtSheet(d.equipLoad.max)}</span>
                <span className="erArrow">→</span>
                <span className={`erVal erValNext${trendClass(d.equipLoad.max, dp.equipLoad.max)}`}>
                  {fmtSheet(dp.equipLoad.max)}
                </span>
              </div>
              <div className="erKvRow">
                <span className="erLabel">Light load</span>
                <span className="erVal erValCur">
                  {fmtSheet(d.equipLoad.light)}
                </span>
                <span className="erArrow">→</span>
                <span className="erVal erValNext">
                  {fmtSheet(dp.equipLoad.light)}
                </span>
              </div>
              <div className="erKvRow">
                <span className="erLabel">Med load</span>
                <span className="erVal erValCur">
                  {fmtSheet(d.equipLoad.medium)}
                </span>
                <span className="erArrow">→</span>
                <span className="erVal erValNext">
                  {fmtSheet(dp.equipLoad.medium)}
                </span>
              </div>
            </div>

          </section>

          <section className="erCol erColRight">
            <h2 className="erColTitle">Defense power</h2>
            <div className="erKv">
              <div className="erKvRow">
                <span className="erLabel">Physical</span>
                <span className="erVal erValCur">{fmtSheet(d.defense.physical)}</span>
                <span className="erArrow">→</span>
                <span className={`erVal erValNext${trendClass(d.defense.physical, dp.defense.physical)}`}>
                  {fmtSheet(dp.defense.physical)}
                </span>
              </div>
              <div className="erKvRow">
                <span className="erLabel">Magic</span>
                <span className="erVal erValCur">{fmtSheet(d.defense.magic)}</span>
                <span className="erArrow">→</span>
                <span className={`erVal erValNext${trendClass(d.defense.magic, dp.defense.magic)}`}>
                  {fmtSheet(dp.defense.magic)}
                </span>
              </div>
              <div className="erKvRow">
                <span className="erLabel">Fire</span>
                <span className="erVal erValCur">{fmtSheet(d.defense.fire)}</span>
                <span className="erArrow">→</span>
                <span className={`erVal erValNext${trendClass(d.defense.fire, dp.defense.fire)}`}>
                  {fmtSheet(dp.defense.fire)}
                </span>
              </div>
              <div className="erKvRow">
                <span className="erLabel">Lightning</span>
                <span className="erVal erValCur">{fmtSheet(d.defense.lightning)}</span>
                <span className="erArrow">→</span>
                <span className={`erVal erValNext${trendClass(d.defense.lightning, dp.defense.lightning)}`}>
                  {fmtSheet(dp.defense.lightning)}
                </span>
              </div>
              <div className="erKvRow">
                <span className="erLabel">Frost</span>
                <span className="erVal erValCur">
                  {fmtSheet(d.defense.frost)}
                </span>
                <span className="erArrow">→</span>
                <span className="erVal erValNext">
                  {fmtSheet(dp.defense.frost)}
                </span>
              </div>
              <div className="erKvRow">
                <span className="erLabel">Poison</span>
                <span className="erVal erValCur">
                  {fmtSheet(d.defense.poison)}
                </span>
                <span className="erArrow">→</span>
                <span className="erVal erValNext">
                  {fmtSheet(dp.defense.poison)}
                </span>
              </div>
            </div>

            <h2 className="erColTitle erColTitleSpaced">Body</h2>
            <div className="erKv">
              <div className="erKvRow">
                <span className="erLabel">Immunity</span>
                <span className="erVal erValCur">{fmtSheet(d.thresholds.immunity)}</span>
                <span className="erArrow">→</span>
                <span className={`erVal erValNext${trendClass(d.thresholds.immunity, dp.thresholds.immunity)}`}>
                  {fmtSheet(dp.thresholds.immunity)}
                </span>
              </div>
              <div className="erKvRow">
                <span className="erLabel">Robustness</span>
                <span className="erVal erValCur">{fmtSheet(d.thresholds.robustness)}</span>
                <span className="erArrow">→</span>
                <span className={`erVal erValNext${trendClass(d.thresholds.robustness, dp.thresholds.robustness)}`}>
                  {fmtSheet(dp.thresholds.robustness)}
                </span>
              </div>
              <div className="erKvRow">
                <span className="erLabel">Focus</span>
                <span className="erVal erValCur">{fmtSheet(d.thresholds.focus)}</span>
                <span className="erArrow">→</span>
                <span className={`erVal erValNext${trendClass(d.thresholds.focus, dp.thresholds.focus)}`}>
                  {fmtSheet(dp.thresholds.focus)}
                </span>
              </div>
              <div className="erKvRow">
                <span className="erLabel">Vitality</span>
                <span className="erVal erValCur">{fmtSheet(d.thresholds.vitality)}</span>
                <span className="erArrow">→</span>
                <span className={`erVal erValNext${trendClass(d.thresholds.vitality, dp.thresholds.vitality)}`}>
                  {fmtSheet(dp.thresholds.vitality)}
                </span>
              </div>
              <div className="erKvRow">
                <span className="erLabel">Madness</span>
                <span className="erVal erValCur">{fmtSheet(d.thresholds.madness)}</span>
                <span className="erArrow">→</span>
                <span className={`erVal erValNext${trendClass(d.thresholds.madness, dp.thresholds.madness)}`}>
                  {fmtSheet(dp.thresholds.madness)}
                </span>
              </div>
            </div>
          </section>
        </main>

        <footer className="erFooter">
          <div className="erFooterHints">
            <span>
              <kbd className="erKbd">A</kbd> OK
            </span>
            <span>
              <kbd className="erKbd">B</kbd> Back
            </span>
            <span>
              <kbd className="erKbd">RS</kbd> Simple view
            </span>
            <span>
              <kbd className="erKbd">Sel</kbd> Help
            </span>
          </div>
          {mockPreview ? (
            <p className="erMockBadge">Mock preview — add <code>?mock=1</code> or run Vite dev</p>
          ) : null}
        </footer>
      </div>
    </div>
  );
}
