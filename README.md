# Simplified Hamm Algorithm for Frequent Itemset Mining

## 📌 Overview
This project implements a simplified version of the **Hamm algorithm**, originally presented in the TKDE 2023 paper:

> *"Mining High Utility Itemsets Using Prefix Trees and Utility Vectors"*

While the original Hamm algorithm targets **High-Utility Itemset Mining (HUIM)**, this implementation adapts its core **TV Structure (Tree + Vector)** to solve the classic **Frequent Itemset Mining (FPM)** problem.

By setting:

- **Unit Price = 1**  
- **Quantity = 1**

we treat:

> **Utility = Frequency (Support)**

---

## 🚀 Quick Start

### 1. Environment Setup
We provide a setup script that automates:

- System dependency checks  
- Python virtual environment creation  
- Library installation (e.g., `psutil`)  
- C++ core compilation  

```bash
chmod +x setup_env.sh
./setup_env.sh
```

---

### 2. Data Preparation
Place your raw transaction datasets in:

```
data_raw/
```

**Supported formats:**
- `.data`
- `.txt`

**Format:**
- Items separated by commas  
- Example:  
```
f,n,t,l,won
```

The system automatically applies **Set Semantics**:
> Duplicate items in the same transaction are removed.

---

### 3. Running Automated Experiments

Activate environment:
```bash
source .venv/bin/activate
```

#### Basic Run
```bash
python3 experiment.py --datasets "car" --minsup-ratios "1,2,5"
```

#### Full Baseline Reproduction
```bash
python3 experiment.py \
  --datasets "mushroom,connect-4,car,kr-vs-kp" \
  --tx-ratios "10,50,100" \
  --minsup-ratios "1,2,5" \
  --override-default-minsup "mushroom=5,connect-4=10" \
  --parallel 4 \
  --resume
```

---

## 🛠️ Implementation Logic

### 1. TV Structure (Tree + Vector)

Uses Hamm’s separation of:

- **Prefix Tree (UP-Tree)**  
  Maintains structural relationships between items.

- **Utility Vector (Support Vector)**  
  Enables fast counting without repeatedly scanning the tree.

---

### 2. Mining Process & Optimization

#### Projection & Traceback
For each item in the header table:

- Trace prefix paths back to root  
- Build conditional vectors  

#### Single Node Optimization
If a tree is a **single linear path**:

- Skip recursive mining  
- Directly generate all subsets via combinations  

This significantly reduces time complexity.

---

## 📋 Argument Reference (`experiment.py`)

| Argument | Description | Example |
|---------|------------|---------|
| `--datasets` | Dataset names in `data_raw/` | `mushroom,car` |
| `--tx-ratios` | Percentage of transactions | `10,50,100` |
| `--tx-size` | Fixed number of transactions | `50000` |
| `--minsup-ratios` | MinSup multipliers | `0.5,1,2` |
| `--override-default-minsup` | Dataset-specific base minsup | `mushroom=5` |
| `--parallel` | Number of parallel processes | `4` |
| `--resume` | Skip completed experiments | `--resume` |

---

## 📊 Visualization

To visualize the FP-Tree structure:

```bash
python3 tools/visualize_fptree.py data_raw/car.data --minsup 10 --limit 20
```

Output:
```
fptree_viz.png
```

Shows:
- Tree branching structure  
- Node frequencies  

Useful for debugging and analysis.

---

## 📁 Project Structure

```
src/Hamm.cpp           # Core C++ implementation
tools/hamm             # Compiled executable
experiment.py          # Python experiment runner
setup_env.sh           # Environment & build script
data_raw/              # Input datasets
results/               # Output patterns & performance logs
```

---

## 🧠 Conceptual Summary

> This project demonstrates that the **TV-Structure proposed in Hamm is not limited to HUIM**,  
> but can be simplified into a highly efficient **Frequent Pattern Mining framework**,  
> conceptually bridging **FP-Growth and utility-based mining**.
