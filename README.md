# Simplified Hamm Algorithm for Frequent Itemset Mining

## 📌 Overview
This project implements a simplified version of the **Hamm algorithm**, originally presented in the TKDE 2023 paper:

> *"Mining High Utility Itemsets Using Prefix Trees and Utility Vectors"*

While the original Hamm algorithm targets **High-Utility Itemset Mining (HUIM)** (considering profit and quantity), this implementation adapts its **core data structure — the TV Structure (Tree + Vector)** to solve the classic **Frequent Itemset Mining (FPM)** problem.

---

## 🔑 Key Concept
In this simplified version:

- Every item has:
  - **Unit price = 1**
  - **Quantity = 1**

Therefore:

> **Utility = Frequency (Support)**

The entire algorithm becomes a **frequency-based pattern mining method** using Hamm’s structural ideas.

---

## 🚀 Features

- **TV Structure Implementation**  
  Uses Hamm’s separation of:
  - **Prefix Tree** → structure
  - **Vector** → counting

- **Single Node Optimization**  
  Detects linear paths (no branches) and generates all combinations directly using combinatorics.

- **High Performance (C++)**  
  Implemented in C++ for:
  - Efficient memory usage  
  - Fast pointer-based tree traversal

---

## 🛠️ Implementation Logic

### 1. Simplification Strategy

To adapt HUIM → FPM, the following are removed:

- ❌ Ignored:
  - TWU (Transaction Weighted Utility)
  - RU (Remaining Utility)
  - Negative utilities

- ✅ Modified:
  - Utility Vector → simple **Support Count Vector**

---

### 2. Algorithm Workflow

#### Step 1: Preprocessing
- Count frequency of all 1-items  
- Filter items where `frequency < min_sup`  
- Sort remaining items in **descending frequency order**  
- Reorder each transaction using this order  

---

#### Step 2: Tree Construction (UP-Tree)
Build a compact prefix tree based on the revised transactions.

**Node Structure:**
Unlike the original Hamm structure which stores `brau` (branch utility) and `upos` (utility vector pointer), our simplified node stores:
- **Item ID**: The identifier of the item.
- **Count**: The support frequency of the path (Mapped from `brau` in the original paper, as Utility = Frequency in this simplified scope).
- **Parent Pointer**: To trace the prefix path during mining.
- **Child Pointers**: To maintain the tree structure.
- **Node Link**: A pointer to the next node with the same Item ID (used for the Header Table).

---

#### Step 3: Mining Process
For each item (bottom-up in header table):

1. **Projection**  
   Find all nodes of this item.

2. **Vector Construction**  
   Trace prefix paths back to root.

3. **Counting**  
   Aggregate supports from paths.

4. **Recursion**  
   Generate longer patterns:
   - `{A}`
   - `{A, B}`
   - `{A, B, C}`

---

#### Step 4: Single Node Optimization
If a tree becomes a **single path (no branches)**:

- Skip recursion
- Directly output all subsets using combinations:
  - `{A}`
  - `{B}`
  - `{A, B}`
  - `{A, C}`
  - ...

This avoids building unnecessary trees.

---

## 📋 Input & Output

### Input Format
The program accepts a **transaction database file (text format)**.

- Each line represents a **single transaction**
- Items are represented by **integers**
- Items are separated by **spaces**
- **Do NOT include transaction IDs**

#### Example (`data.txt`)
```text
1 2 3
1 2
1
2 3
```

---

### Output Format
The program outputs **all Frequent Itemsets** that meet the minimum support threshold, followed by their **support count**.

**Format:**
[Item 1] [Item 2] ... : [Support Count]

Example Output:
```text
3 : 2
2 3 : 2
2 : 3
1 2 : 2
1 : 3
```