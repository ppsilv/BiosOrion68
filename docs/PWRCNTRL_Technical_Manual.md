# PWRCNTRL Technical Manual
## Power Sequencing & Supervisory Controller for the PSU_PDS317 Backplane

**Document type:** Technical Manual
**Firmware target:** Arduino Nano (ATmega328P)
**Revision:** 1.0
**Status:** Field-tested, production candidate

---

## 1. Overview

PWRCNTRL is the firmware running on the Arduino Nano that supervises the
three DC rails (+12V, +5V, −5V) of the PSU_PDS317 power supply and
controls their connection to the PDS317 CPU backplane. It performs four
core functions:

1. **Sequencing** — holds each rail disconnected from the backplane until
   its voltage has stabilized at a safe level.
2. **Supervision** — continuously monitors each rail and reports a
   Power-Good (PWRG) status per rail to the host CPU.
3. **Fault recovery** — enforces a configurable stabilization hold-off
   before re-asserting Power-Good after any rail dips out of tolerance,
   preventing status "chatter" that could put the CPU into a boot/fault
   loop.
4. **Safe shutdown** — guarantees that CPU-facing relays are opened
   *before* the upstream mains relay removes power from the supplies,
   so the CPU never observes a live rail collapsing under it.

The system is controlled by a single physical ON/OFF pushbutton and
reports status via a 20×4 I2C character LCD and a serial debug console.

---

## 2. Hardware Interface

### 2.1 Pin Assignment

| Pin | Direction | Name | Function |
|---|---|---|---|
| A0 | Analog in | `PIN_ADC_12V` | +12V rail sense (scaled 0–12V → 0–5V) |
| A1 | Analog in | `PIN_ADC_5V` | +5V rail sense (1:1, no scaling) |
| A2 | Analog in | `PIN_ADC_NEG5V` | −5V rail sense (front-end outputs a positive magnitude, 0–5V, proportional to the −5V rail) |
| D2 | Digital out | `PIN_RELAY_12V` | +12V rail-to-backplane relay driver |
| D3 | Digital out | `PIN_RELAY_5V` | +5V rail-to-backplane relay driver |
| D4 | Digital out | `PIN_RELAY_NEG5V` | −5V rail-to-backplane relay driver |
| D5 | Digital out | `PIN_ANY_RELAY_ON` | Aggregate indicator — HIGH if at least one rail relay is engaged |
| D6 | Digital in | `PIN_CPU_FEEDBACK` | CPU-initiated cut request, active LOW, debounced (Section 6) |
| D7 | — | *(unused)* | Reserved / not wired; LCD uses A4/A5 (I2C), not a dedicated digital pin |
| D8 | Digital out | `PIN_PWRG_12V` | +12V Power-Good signal to CPU |
| D9 | Digital out | `PIN_PWRG_5V` | +5V Power-Good signal to CPU |
| D10 | Digital out | `PIN_PWRG_NEG5V` | −5V Power-Good signal to CPU |
| D11 | Digital in | `PIN_KEY_POWER_ON` | Physical ON/OFF pushbutton, debounced, edge-triggered toggle |
| A4 | I2C SDA | — | LCD (PCF8574 backpack) |
| A5 | I2C SCL | — | LCD (PCF8574 backpack) |
| D7 (mains) | Digital out | `PIN_RELAY_POWER_ON` | Drives the upstream "mains" relay pair (via a single transistor, two coils in parallel) that applies input power to the PSU modules themselves |

> **Note on rail relay indicator LEDs:** each rail relay is fitted with a
> local LED for visual "should be energized" confirmation. This is a
> passive hardware indicator; it does not feed back into the firmware
> and does not confirm actual contact closure (see Section 9.3 for a
> discussion of adding true relay feedback).

### 2.2 LCD

- Module: I2C character LCD, 20 columns × 4 rows (PCF8574 backpack)
- Default I2C address: `0x27`. Some backpacks ship at `0x3F` — run an I2C
  bus scanner if the display does not respond.
- Refresh interval: 500 ms (`LCD_UPDATE_MS`), non-blocking (uses `millis()`,
  not `delay()`).

### 2.3 Serial Debug Console

- Baud rate: 115200
- All diagnostic output is prefixed `[DEBUG]` and is edge-triggered
  where applicable (see Section 6) to avoid flooding the console during
  a sustained condition.

---

## 3. Voltage Sensing and Calibration

Each rail's raw ADC reading is converted to an actual voltage using:

```
V_real = (ADC_raw * 5.0 / 1023.0) * SCALE + OFFSET
```

| Rail | Scale | Offset | Rationale |
|---|---|---|---|
| +12V | 12.0 / 5.0 | 0.0 | Resistive divider maps 0–12V to the Nano's 0–5V ADC input range |
| +5V | 1.0 | 0.0 | No divider needed; rail is already within ADC range |
| −5V | 1.0 | 0.0 | Analog front-end outputs a **positive** voltage (0–5V) proportional to the magnitude of the −5V rail. The firmware treats this channel's nominal as a positive value (5.0) and works in magnitude; the LCD multiplies the displayed value by −1 for human-readable sign only. |

All calibration constants (`SCALE_12V`, `OFFSET_12V`, `SCALE_5V`,
`OFFSET_5V`, `SCALE_NEG5V`, `OFFSET_NEG5V`) are `const float` values near
the top of the firmware and must be re-derived if the analog front-end
(divider resistor values, op-amp gain, etc.) changes.

Each voltage reading is the average of 8 consecutive `analogRead()`
samples, taken back-to-back with no inter-sample delay, to reduce ADC
noise before the reading is used in any threshold decision.

---

## 4. Rail State Machine

Each of the three rails (+12V, +5V, −5V) is represented by an independent
`Rail` structure and runs its own copy of the following state machine —
**there is no shared or cross-rail timing state**; a fault or recovery on
one rail has no effect on the timers of the other two.

### 4.1 States and Transitions

| State | Meaning |
|---|---|
| Relay open, `hadFault = false` | Cold start — rail has never been engaged since the last full power-off or firmware reset |
| Relay open, `hadFault = true` | Rail was cut *during operation* (e.g., by a CPU-initiated request) and has not yet been re-qualified |
| Relay closed, PWRG asserted | Normal operating state |
| Relay closed, PWRG de-asserted | Rail voltage sagged below the fail threshold; relay remains closed, CPU is informed via PWRG only |

### 4.2 Turn-On Qualification

When a rail's relay is open and its voltage reaches **95% of nominal**
(`PCT_ON`), the firmware starts a qualification timer. The voltage must
remain continuously at or above 95% for a required hold time before the
relay is closed and PWRG is asserted. Any dip below 95% during this
window resets the timer to zero (no partial credit is accumulated).

The **required hold time is context-dependent**:

- **`QUALIFY_MS` (100 ms)** — used for a genuine cold start (`hadFault ==
  false`). This is intentionally short so the system reaches operating
  state quickly on normal power-up.
- **`RECOVERY_HOLD_MS` (10,000 ms / 10 s, configurable)** — used whenever
  `hadFault == true`, i.e., the rail is being re-qualified after having
  been cut *during* operation rather than through a clean full
  power-off. This asymmetry is deliberate: a fault recovery deserves far
  more caution than a first-time power-up, and using the fast path here
  would defeat the purpose of the anti-chatter design described in
  Section 4.4.

### 4.3 Power-Good De-assertion (Fail Detection)

While a rail is active (relay closed, PWRG asserted), if its voltage
drops below **90% of nominal** (`PCT_OFF`) continuously for `QUALIFY_MS`
(100 ms), PWRG is de-asserted. **By design, the relay itself remains
closed** — this transition only informs the CPU of a marginal
condition; it does not disconnect the rail. This behavior is
intentional and documented; if a design change requires the relay to
open on this condition as well, the single line to modify is called out
in the firmware comments at that block.

### 4.4 Power-Good Recovery (Anti-Chatter Hold-Off)

This is the most safety-critical piece of logic in the firmware and the
result of an iterative fix during bring-up testing.

**Problem observed during testing:** if a supply's output hovers near the
95%/90% boundary (bench supply noise, marginal regulation, etc.), a naive
implementation re-asserts PWRG the instant a single ADC reading crosses
95% again. If the voltage is oscillating near that boundary, PWRG toggles
on and off every loop iteration, and a CPU watching that signal can enter
a rapid power-good/power-fail loop.

**Fix:** recovery from a PWRG-failed state requires the same
`RECOVERY_HOLD_MS` (10 s default) of **continuous** stability at or above
95% before PWRG is re-asserted. If the voltage dips below 95% at any
point during that window, the hold-off timer resets to zero — partial
stability is never credited.

```
Rail voltage:  ────╲___________╱───────────────────────
PWRG:          ─────┐                                  ┌────
                     └──────────────────────────────────┘
                     ↑                                  ↑
                fails <90%                    RECOVERY_HOLD_MS of
                                               continuous ≥95% required
                                               before PWRG re-asserts
```

### 4.5 Unified Hold-Off Regardless of Cause (`hadFault`)

A second, related defect was found during testing: a rail cut via
`PIN_CPU_FEEDBACK` (Section 6) sets `relayOn = false`, which routes the
*next* re-qualification through the turn-on block (Section 4.2) rather
than the recovery block (Section 4.4) — and the turn-on block originally
used only the fast 100 ms `QUALIFY_MS`, bypassing the intended 10-second
caution period entirely.

The `hadFault` flag closes this gap: any rail cut while the system
remains powered (CPU feedback request) sets `hadFault = true`, which
forces the turn-on block itself to require `RECOVERY_HOLD_MS` instead of
`QUALIFY_MS` the next time it re-qualifies. `hadFault` is cleared:

- automatically, once the rail successfully re-qualifies, or
- unconditionally, on a full clean power-off via the pushbutton (Section
  5), since that is a legitimate cold restart, not a fault.

---

## 5. Power Button and Safe Shutdown Sequence

### 5.1 Button Debounce and Toggle

`PIN_KEY_POWER_ON` is read every loop iteration through a debounce filter
(`DEBOUNCE_MS = 100 ms`). The system toggles `outputState` **only on the
falling edge** (HIGH→LOW, i.e., the moment the button is pressed) of the
debounced signal — never on release, and never more than once per press,
regardless of how long the button is held.

### 5.2 Power-On Sequence

1. User presses the button; `outputState` becomes `true`.
2. `PIN_RELAY_POWER_ON` is driven HIGH, energizing the two parallel
   mains-relay coils (via a single transistor) that apply input power to
   the PSU modules.
3. The PSU modules begin ramping their outputs. `updateRail()` resumes
   normal evaluation (see Section 5.4) and each rail proceeds through the
   turn-on qualification described in Section 4.2.

### 5.3 Power-Off Sequence — Safety-Critical Ordering

This sequence was corrected during bring-up after testing revealed a
dangerous defect (see Section 5.4 for the root cause). The **current,
correct** sequence, executed atomically within the button handler, is:

1. `outputState` becomes `false`.
2. **`resetRailInternalState()` is called for all three rails
   immediately** — each rail's relay pin and PWRG pin are driven LOW,
   and all internal state (`relayOn`, `pwrgOn`, pending timers,
   `hadFault`) is cleared. **This disconnects the CPU-facing relays
   before anything else happens.**
3. Only after step 2 completes does `PIN_RELAY_POWER_ON` go LOW, opening
   the mains relay and removing input power from the PSU modules.

This ordering guarantees the CPU is already electrically isolated from
the backplane rails before those rails begin to sag from capacitor
discharge — the CPU never observes a live, collapsing supply.

### 5.4 Root Cause of the Original Shutdown Defect (Historical Record)

During testing, it was observed that after commanding a power-off, the
rails appeared to remain connected to the CPU as their voltage decayed —
an unsafe condition. Investigation found that step 2 above **was already
correct** (relays opened immediately), but a second, independent
mechanism was silently re-closing them within roughly 100 ms:

`updateRail()` runs unconditionally every loop iteration and has no
inherent awareness of whether the system has just been commanded off.
Immediately after the mains relay opens, the PSU output capacitors have
not yet discharged, so the sensed voltage remains above the 95%
threshold for a brief period. Since `resetRailInternalState()` had
already cleared `hadFault` to `false`, the turn-on block evaluated this
residual voltage using the fast `QUALIFY_MS` (100 ms) path and
re-energized the relay — reconnecting the CPU in the middle of the
supply's decay.

**Fix:** `updateRail()` now begins with an unconditional guard:

```cpp
if (!outputState) {
    return;
}
```

placed immediately after the voltage is sampled and stored (so the LCD
still displays the decaying voltage), but before any state-machine
evaluation runs. This guarantees that once the system is commanded off,
no rail can be re-armed by residual voltage — only an explicit new
button press (a genuine new power-on event) can bring a rail back.

---

## 6. CPU Feedback Input

`PIN_CPU_FEEDBACK` (D6) allows the host CPU to request that any currently
faulted rail (PWRG de-asserted) be actively disconnected, rather than
merely flagged. The protocol is intentionally minimal in this revision:

- **Active LOW.**
- While asserted, any rail with `relayOn == true && pwrgOn == false` has
  its relay opened and `hadFault` set (routing its next recovery through
  the full `RECOVERY_HOLD_MS`, per Section 4.5).
- This is a **single-bit, system-wide** signal in the current design — it
  does not distinguish which rail the CPU is reacting to, nor the
  reason. A future revision could replace this with a structured
  protocol (e.g., I2C) for per-rail granularity.

### 6.1 Debounce

The raw pin is filtered through `CPU_FEEDBACK_DEBOUNCE_MS` (30 ms,
configurable) using the same debounce technique as the power button: the
level must remain stable for the full window before being accepted as
real. This was added after bench testing confirmed the input is
susceptible to relay-switching noise on the same board; both the fault
logic and the LCD status readout consume the filtered value exclusively,
so the two never disagree.

### 6.2 Debug Logging

Console logging for this input is **edge-triggered**: a message is
printed only on the transition to active and the transition back to
inactive, not once per loop iteration while the condition is held. This
was a deliberate refinement after initial testing produced continuous
repeated log lines while the signal was held low for an extended period
during a manual test.

---

## 7. LCD Layout Reference

```
Col:  0         1         2
      0123456789012345678901234567890
Row0: 12V:xx.xx  <status>
Row1:  5V: xx.xx  <status>
Row2:     -5V:xx.xx      <status>
Row3:     CPU FB:<ATIVO|----->  <LIGADO|DESLIG>
```

- `<status>` per rail is one of: `PWR down` (voltage below 1.0V — supply
  is off or has fully discharged), `OK` (PWRG asserted), or `FAIL` (PWRG
  de-asserted, relay still closed per Section 4.3).
- Row 3, right side, mirrors `outputState` (`LIGADO` = system commanded
  on, `DESLIG` = commanded off) — this is the same value shown
  immediately at the moment the button is pressed and refreshed on the
  next periodic LCD cycle.

---

## 8. Configuration Reference

All tunable constants are grouped near the top of the firmware file.

| Constant | Default | Purpose |
|---|---|---|
| `PCT_ON` | 0.95 | Fraction of nominal voltage required to qualify a rail as good |
| `PCT_OFF` | 0.90 | Fraction of nominal voltage below which PWRG is de-asserted |
| `QUALIFY_MS` | 100 ms | Continuous-stability time required for a cold turn-on |
| `RECOVERY_HOLD_MS` | 10,000 ms | Continuous-stability time required after any in-operation fault (voltage sag recovery, or CPU-requested cut recovery) |
| `DEBOUNCE_MS` | 100 ms | Power button debounce window |
| `CPU_FEEDBACK_DEBOUNCE_MS` | 30 ms | CPU feedback input debounce window |
| `LCD_UPDATE_MS` | 500 ms | LCD (and serial debug) refresh interval |

**Tuning guidance:** `RECOVERY_HOLD_MS` should always remain significantly
larger than `QUALIFY_MS` — the asymmetry is what prevents the anti-chatter
mechanism (Section 4.4) from being bypassed. There is currently a single
global `RECOVERY_HOLD_MS` shared by all three rails; per-rail values are
a straightforward future extension if the three PSU modules exhibit
meaningfully different recovery characteristics (see Section 9.1).

---

## 9. Known Limitations and Suggested Future Work

These are documented gaps, not defects — the system is safe as shipped,
but the following would improve robustness or diagnostics further.

### 9.1 Per-Rail Recovery Timing
`RECOVERY_HOLD_MS` is currently a single constant shared by all three
rails. If the +12V/−5V auxiliary supplies and the +5V main supply have
meaningfully different settling characteristics, moving this constant
into the `Rail` structure (one value per rail) would allow independent
tuning without affecting the others.

### 9.2 Watchdog Timer
The Arduino Nano is the sole authority gating power to the CPU backplane.
Enabling the AVR's internal watchdog timer would ensure that a firmware
hang (from any future code change) results in a reset rather than a
stuck state. Since `outputState` initializes to `false` on boot, a
watchdog-triggered reset would naturally fail safe (system starts fully
de-energized).

### 9.3 Relay Contact Confirmation
The rail relays currently have local LED indicators for a visual "should
be energized" check, but the firmware has no electrical confirmation
that a commanded relay closure actually occurred (e.g., via an auxiliary
relay contact wired back to a spare digital input). This would catch a
mechanically stuck relay before the CPU is powered up expecting a rail
that never actually connected.

### 9.4 Structured CPU Feedback Protocol
The current CPU feedback interface is a single active-low line with no
per-rail addressing. A future revision could replace this with an I2C or
UART-based protocol carrying rail identity and a reason code, allowing
more precise CPU-driven power management.

### 9.5 Non-Volatile Fault Logging
The ATmega328P's internal EEPROM (1 KB) is unused in the current
firmware. Logging the rail, event type, and an approximate timestamp of
each fault would allow post-mortem diagnosis of intermittent issues
without requiring a serial console to be attached at the time of the
event.

### 9.6 Temperature Monitoring
Provision for three DS18B20 1-Wire temperature sensors (one per rail) is
part of the broader PSU_PDS317 hardware plan but is not yet implemented
in this firmware revision.

---

## 10. Revision History

| Rev | Description |
|---|---|
| 0.1 | Initial rail hysteresis logic (turn-on qualification, PWRG fail on sag) |
| 0.2 | Added instantaneous PWRG recovery — later identified as causing status chatter near the threshold boundary under noisy/marginal supply conditions |
| 0.3 | Introduced `RECOVERY_HOLD_MS` continuous-stability hold-off on PWRG recovery |
| 0.4 | Discovered and fixed the CPU-feedback-cut bypass: added `hadFault` to unify hold-off duration regardless of which code path re-qualifies a rail |
| 0.5 | Diagnosed sustained-LOW condition on `PIN_CPU_FEEDBACK` during bench testing (traced to intentional manual test, not a wiring fault) |
| 0.6 | Fixed critical shutdown-sequencing defect: rails could be silently re-energized within ~100 ms of a commanded power-off due to residual capacitor voltage; added the `outputState` guard in `updateRail()` |
| 0.7 | Added debounce filtering to `PIN_CPU_FEEDBACK`; unified all reads of that pin (logic and LCD) through the filtered accessor |
| 0.8 | Converted `PIN_CPU_FEEDBACK` debug logging from per-iteration to edge-triggered |
| 0.9 | Resolved Arduino IDE automatic function-prototype generation defect (`'Rail' does not name a type`) by adding explicit manual prototypes after the `Rail` struct definition |
| 1.0 | Current revision — field-tested, all known safety-critical defects resolved |

---

*This manual describes firmware behavior as implemented and verified
through iterative bench testing. Electrical values (rail tolerances,
qualification timing) reflect the defaults in the current firmware and
should be re-validated against final PSU_PDS317 hardware characteristics
before production deployment.*
