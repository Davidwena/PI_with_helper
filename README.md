# Efficient and High-Accuracy Private CNN Inference with Helper-Assisted Malicious Security
This repository contains the proof-of-concept implementation accompanying our paper:

> **Efficient and High-Accuracy Private CNN Inference with Helper-Assisted Malicious Security**

We propose a maliciously secure private CNN inference framework in the **Helper-Assisted Malicious Secure Dishonest-Majority (HA-MSDM)** model, which leverages a semi-trusted helper to accelerate secure multi-party computation while preserving security against a malicious adversary corrupting any subset of the parties.

---

## ⚠️ Repository Status

> **This is a preview release.** The code currently provided in this repository is intended to demonstrate the feasibility of the core protocols and reproduce a subset of the experiments reported in the paper. **The full source code, including end-to-end private inference on all benchmark networks, will be released publicly once the paper is accepted.**

---

## 📦 What Is Included

The current release provides the following components:

### 1. Communication Infrastructure

- Basic networking primitives between the **helper** and the computing **parties**.
- Abstractions for point-to-point channels, broadcast, and helper-to-party correlated randomness delivery.
- Support for LAN / WAN network simulation via Linux `tc`.

### 2. Core Protocols

Implementations of the building-block protocols used throughout the paper:

- **π_input** — authenticated secret sharing of private inputs.
- **π_output** — opening of shares with MAC verification.
- **π_mul** — maliciously secure multiplication using helper-generated correlated randomness.
- **π_mul_trunc** — multiplication with truncation, supporting fixed-point arithmetic over Z_{2^k}.
- Utility routines for MAC generation, share refresh, and consistency checking.

### 3. Setup Phase

- Reference implementation of the **offline / setup phase**, including:
  - Helper-side generation of authenticated Beaver triples and truncation pairs.
  - Distribution of correlated randomness to the parties.

### 4. Partial Experiments

Client-side (party-side) code for a selection of the experiments reported in the paper:

- **Average Pooling** layer under HA-MSDM.
- **Batch Normalization** layer under HA-MSDM.
- **2D Convolution** layer under HA-MSDM.
- **Fixed-point polynomial evaluation** (π_poly_fixed) for non-linear activation approximation.

