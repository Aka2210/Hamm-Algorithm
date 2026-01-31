# Simplified Hamm Algorithm for Frequent Itemset Mining

## 📌 Overview
This project implements a **simplified version of the Hamm algorithm**, originally presented in the **TKDE 2023** paper:

> *"Mining High Utility Itemsets Using Prefix Trees and Utility Vectors"*

This implementation adapts Hamm's **Single Node Optimization** and **FP-tree in FP-Growth** to solve the classic **Frequent Itemset Mining (FPM)** problem.

---

## 🚀 Quick Start

### 1. Environment Setup
We provide a setup script that automates:

- System dependency checks (`g++`)
- Python virtual environment creation
- Library installation (`psutil` for resource monitoring)
- C++ core compilation

```bash
chmod +x setup_env.sh
./setup_env.sh
```

---

### 2. Data Preparation
Place your raw transaction datasets in `data_raw/`.

**Supported formats:** `.data`, `.txt`  
**Format:** Items separated by commas (e.g., `f,n,t,l,won`)

The system automatically applies **Set Semantics**:
- Duplicate items in the same transaction are merged/removed before tree construction.

---

### 3. Running Automated Experiments
Activate the environment:

```bash
source .venv/bin/activate
```

#### Full Baseline Reproduction
Run all datasets with various ratios and parallel processes:

```bash
python3 experiment.py \
  --datasets "mushroom,connect4,car,kr-vs-kp" \
  --tx-ratios "10,50,100" \
  --minsup-ratios "1,2,5" \
  --override-default-minsup "mushroom=5,connect4=10" \
  --parallel 4 \
  --resume
```

---

## 🛠️ Implementation Logic

### 1. Standard FP-tree
Standard FP-Tree Structure Instead of the TV structure in the paper, this implementation utilizes the standard FP-Tree (as defined in the classic FP-Growth algorithm) to maintain itemset information, ensuring memory efficiency while adopting Hamm-style algorithmic flows.

---

### 2. Mining Process & Optimization

#### FP-Growth Based Mining
The core mining process follows standard FP-Growth recursion:

1. **Header Table Traversal**: Mining items from lowest to highest frequency.
2. **Conditional Pattern Base**: Tracing prefix paths to build conditional FP-Trees.
3. **Conditional FP-Tree Construction**: Mining recursively from each conditional tree.

---

#### Single Node Optimization (Hamm's Contribution)
To avoid excessive recursion, we integrate Hamm’s specific optimization:

- **Detection**: If a conditional tree (or the initial tree) is a **single linear path** (no branches), the algorithm stops recursion.
- **Combinatorial Output**: All frequent itemsets are generated directly from the path using combinations.

✅ This hybrid approach ensures the correctness of FP-Growth while gaining the speed of Hamm in dense datasets or deep trees.

---

## 📋 Argument Reference (`experiment.py`)

| Argument | Description | Example |
|---|---|---|
| `--datasets` | Dataset names in `data_raw/` | `mushroom,car` |
| `--tx-ratios` | Percentage of transactions to use | `10,50,100` |
| `--tx-size` | Fixed number of transactions (overrides ratios) | `50000` |
| `--minsup-ratios` | Multipliers for base support | `0.5,1,2` |
| `--override-default-minsup` | Dataset-specific base minsup (%) | `mushroom=5` |
| `--parallel` | Number of parallel processes | `4` |
| `--resume` | Skip already completed experiments | `--resume` |

---

## 📁 Project Structure

```text
src/Hamm.cpp          # Core C++ implementation (FP-Growth + Hamm Opt)
tools/hamm            # Compiled executable
experiment.py         # Python experiment runner (with memory check)
setup_env.sh          # Environment & build script
data_raw/             # Input datasets (.data or .txt)
results/              # Output patterns & performance logs
```

---

## 🧠 Conceptual Summary
This project demonstrates that the **Single Node Optimization** proposed in **Hamm** is highly effective for **Frequent Pattern Mining**.
By integrating these into an **FP-Growth** framework, we achieve a balance between:

- Classical recursive mining (FP-Growth)
- Modern utility-based mining optimizations (Hamm)

---
