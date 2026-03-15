# Cache Side-Channel Attack Demo (Flush+Reload vs Browser Prime+Probe)

## Overview

This project demonstrates a **cache side-channel attack** in two environments:

1. **Native implementation (C++)** – A reliable **Flush+Reload** attack using shared memory and CPU cache instructions.
2. **Browser experiment (JavaScript)** – A conceptual **Prime+Probe-style attack** demonstrating the same principle but with significant noise due to modern browser defenses.

The goal is to show how **microarchitectural behavior of CPU caches can leak secret information**, and why browsers implement strong mitigations against such attacks.

---

# Concept

Modern CPUs use multiple cache levels (L1, L2, L3) to speed up memory access.

Typical access latency:

| Memory Level | Approx Latency |
| ------------ | -------------- |
| L1 cache     | ~4 cycles      |
| L2 cache     | ~12 cycles     |
| L3 cache     | ~40 cycles     |
| Main memory  | ~200+ cycles   |

If an attacker can measure **how long a memory access takes**, they can infer whether the data was already present in cache.

This timing difference forms the basis of **cache side-channel attacks**.

---

# Attack Types Demonstrated

## 1. Flush+Reload (Native C++)

Flush+Reload works when two processes share the **same physical memory page**.

Attack cycle:

```
Attacker flushes cache line
Victim runs and may access the line
Attacker reloads and measures latency
```

Interpretation:

```
Fast reload  → Victim accessed the line
Slow reload  → Victim did not access it
```

 

# Project Structure

```
project/
│
├── attacker.cpp
├── victim.cpp
└── shared_mem.bin
```

---

# Victim Program

The victim repeatedly accesses a secret-dependent memory location.

```cpp
volatile unsigned char *addr = &shared[secret * STRIDE];
*addr;
```

This loads the cache line into the CPU cache.

---

# Attacker Program

The attacker:

1. Flushes all candidate cache lines
2. Waits briefly for victim execution
3. Reloads each cache line
4. Measures latency
5. Infers which line the victim accessed

Key attack logic:

```cpp
_mm_clflush(&shared[i * STRIDE]);
```

followed by

```cpp
uint64_t t = measure(&shared[mix_i * STRIDE]);
```

If the reload time is **below the threshold**, the line was likely accessed by the victim.

---

# Threshold Calibration

The attacker automatically measures:

* **cached access latency**
* **uncached access latency**

```cpp
CACHE_HIT_THRESHOLD = (cached + uncached) / 2;
```

This separates cache hits from misses.

---

# Randomized Probe Order

The attacker probes cache lines in a **mixed order**:

```cpp
mix_i = ((i * 167) + 13) & (NUM_LINES - 1);
```

This prevents CPU **hardware prefetchers** from detecting sequential patterns.

---

# Running the Demo

Compile:

```
g++ attacker.cpp -O2 -march=native -o attacker
g++ victim.cpp -O2 -march=native -o victim
```

Run victim:

```
./victim
```

Run attacker in another terminal:

```
./attacker
```

Example output:

```
Cached latency: 40
Uncached latency: 210
Threshold: 125

Most likely index: 60 hits: 990
```

The attacker successfully infers the victim's secret index.

---

# Browser Version (JavaScript)

A browser-based version of the attack was also implemented using JavaScript.

Instead of Flush+Reload, browsers require a **Prime+Probe-style attack**.

Attack cycle:

```
Prime cache with attacker data
Victim executes
Probe cache and measure timing
```

However, browser attacks are much noisier.

---

# Why Browser Attacks Are Weaker

Modern browsers deliberately degrade microarchitectural side channels.

Key defenses include:

### 1. Timer Precision Reduction

JavaScript timers are coarse.

Typical resolution:

| API               | Precision    |
| ----------------- | ------------ |
| performance.now() | ~100µs – 1ms |

Cache timing differences are **nanoseconds**, so the signal is heavily degraded.

---

### 2. SharedArrayBuffer Restrictions

High-resolution timers using `SharedArrayBuffer` are only enabled under **cross-origin isolation**.

This prevents attackers from easily building precise timers.

---

### 3. Site Isolation

Browsers isolate websites into separate processes.

This reduces cross-site cache sharing.

---

### 4. Spectre Mitigations

After the discovery of **speculative execution attacks**, browsers added protections such as:

* speculation barriers
* bounds checking
* JIT hardening

---

### 5. Prefetcher Interference

Hardware prefetchers can hide cache misses when memory accesses are predictable.

Random probe order is needed to mitigate this.

---

### 6. Browser Noise

Additional sources of noise include:

* OS scheduling
* garbage collection
* JIT compilation
* background tabs
* other processes using the cache

---

# Comparison: Native vs Browser Attacks

| Feature        | Native Flush+Reload  | Browser Prime+Probe |
| -------------- | -------------------- | ------------------- |
| Cache eviction | Precise (`clflush`)  | Indirect            |
| Timing         | Nanosecond (`rdtsc`) | Microsecond         |
| Signal quality | Very strong          | Noisy               |
| Granularity    | Single cache line    | Cache set           |
| Ease of attack | Easier               | Much harder         |

---

# Educational Value

This project demonstrates:

* how CPU caches leak information
* how microarchitectural side channels work
* why modern browsers introduced defenses
* the difference between **native and browser attack models**

---

# Disclaimer

This project is for **educational and research purposes only**.

It demonstrates well-known side-channel techniques used in academic security research.

---

# References

* Flush+Reload: A High Resolution Cache Side-Channel Attack
* Prime+Probe Cache Attacks
* Spectre and Meltdown research papers
* Modern browser security documentation

---
