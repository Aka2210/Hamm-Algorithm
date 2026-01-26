import os
import sys
import argparse
import subprocess
import time
import psutil
import pandas as pd
from concurrent.futures import ProcessPoolExecutor, as_completed

EXE_PATH = "./tools/hamm"
RAW_DATA_DIR = "data_raw"
TEMP_DATA_DIR = "data_temp"
RESULTS_DIR = "results"
MIN_MEMORY_AVAILABLE_GB = 2.0 

os.makedirs(TEMP_DATA_DIR, exist_ok=True)
os.makedirs(RESULTS_DIR, exist_ok=True)

def check_memory():
    while True:
        mem = psutil.virtual_memory()
        available_gb = mem.available / (1024 ** 3)
        if available_gb > MIN_MEMORY_AVAILABLE_GB:
            return
        print(f"⚠️  Low memory ({available_gb:.2f} GB available). Waiting 10s...")
        time.sleep(10)

def preprocess_dataset(dataset_name, tx_ratio=None, tx_size=None):
    raw_path = os.path.join(RAW_DATA_DIR, f"{dataset_name}.data")
    if not os.path.exists(raw_path):
        raw_path = os.path.join(RAW_DATA_DIR, f"{dataset_name}.txt")
        if not os.path.exists(raw_path):
            print(f"❌ Dataset not found: {dataset_name}")
            return None, 0

    with open(raw_path, 'r', encoding='utf-8', errors='ignore') as f:
        lines = f.readlines()
    
    total_len = len(lines)
    target_lines = []

    suffix = ""
    if tx_size is not None:
        count = int(tx_size)
        if count > total_len:
            print(f"⚠️  Requested tx_size {count} > dataset size {total_len}. Using full dataset.")
            count = total_len
        target_lines = lines[:count]
        suffix = f"size{count}"
    elif tx_ratio is not None:
        ratio = float(tx_ratio)
        count = int(total_len * (ratio / 100.0))
        if count == 0: count = 1
        target_lines = lines[:count]
        suffix = f"ratio{ratio}"
    else:
        target_lines = lines
        suffix = "full"

    temp_filename = f"{dataset_name}_{suffix}.tmp"
    temp_path = os.path.join(TEMP_DATA_DIR, temp_filename)
    
    with open(temp_path, 'w', encoding='utf-8') as f:
        f.writelines(target_lines)
        
    return temp_path, len(target_lines)

def run_single_experiment(dataset, input_file, min_sup_rate, output_file):
    check_memory()
    
    cmd = [EXE_PATH, str(min_sup_rate), input_file, output_file]
    
    start_time = time.time()
    try:
        result = subprocess.run(cmd, capture_output=True, text=True)
        duration = time.time() - start_time
        
        if result.returncode != 0:
            return {
                "status": "error",
                "dataset": dataset,
                "msg": result.stderr
            }
        
        return {
            "status": "success",
            "dataset": dataset,
            "min_sup": min_sup_rate,
            "duration": duration
        }
    except Exception as e:
        return {"status": "exception", "msg": str(e)}

def parse_list(s):
    if not s: return []
    return [x.strip() for x in s.split(',')]

def parse_dict(s):
    d = {}
    if not s: return d
    for item in s.split(','):
        if '=' in item:
            k, v = item.split('=')
            d[k.strip()] = float(v.strip())
    return d

def main():
    parser = argparse.ArgumentParser(description="Hamm Algorithm Experiment Runner")
    
    parser.add_argument("--tx-ratios", type=str, default="100", help="List of tx ratios (%), e.g., '10,50,100'")
    parser.add_argument("--minsup-ratios", type=str, default="1", help="List of minsup ratios (%), e.g., '0.5,1,2'")
    parser.add_argument("--datasets", type=str, required=True, help="List of datasets, e.g., 'connect-4,chess'")
    parser.add_argument("--override-default-minsup", type=str, default="", help="Specific minsup for dataset, e.g., 'mushroom=0.5,chess=2'")
    parser.add_argument("--tx-size", type=int, default=None, help="Fixed number of transactions to use (overrides tx-ratios)")
    parser.add_argument("--resume", action="store_true", help="Skip existing result files")
    parser.add_argument("--parallel", type=int, default=4, help="Number of parallel threads (default: 4)")

    args = parser.parse_args()

    tx_ratios = parse_list(args.tx_ratios)
    minsup_ratios = [float(x) for x in parse_list(args.minsup_ratios)]
    datasets = parse_list(args.datasets)
    overrides = parse_dict(args.override_default_minsup)

    tasks = []

    print("🚀 Preparing Experiments...")
    
    for ds in datasets:
        current_minsups = minsup_ratios
        target_minsups = []
        if ds in overrides:
            target_minsups = [overrides[ds]]
        else:
            target_minsups = minsup_ratios

        run_configs = []
        if args.tx_size is not None:
            tmp_file, real_cnt = preprocess_dataset(ds, tx_size=args.tx_size)
            if tmp_file:
                run_configs.append((tmp_file, f"size{args.tx_size}"))
        else:
            for ratio in tx_ratios:
                tmp_file, real_cnt = preprocess_dataset(ds, tx_ratio=ratio)
                if tmp_file:
                    run_configs.append((tmp_file, f"ratio{ratio}"))

        for tmp_file, suffix in run_configs:
            for rate in target_minsups:
                final_rate_arg = rate / 100.0
                
                output_filename = f"{ds}_{suffix}_sup{rate}.txt"
                output_path = os.path.join(RESULTS_DIR, output_filename)

                if args.resume and os.path.exists(output_path) and os.path.getsize(output_path) > 0:
                    print(f"⏩ Skipping {output_filename} (Already exists)")
                    continue

                tasks.append({
                    "dataset": ds,
                    "input": tmp_file,
                    "rate_arg": final_rate_arg,
                    "output": output_path,
                    "display_rate": rate
                })

    print(f"📋 Total Tasks: {len(tasks)}")
    print(f"⚙️  Parallel Workers: {args.parallel}")
    
    with ProcessPoolExecutor(max_workers=args.parallel) as executor:
        futures = []
        for task in tasks:
            f = executor.submit(
                run_single_experiment,
                task['dataset'],
                task['input'],
                task['rate_arg'],
                task['output']
            )
            futures.append(f)
        
        for f in as_completed(futures):
            res = f.result()
            if res['status'] == 'success':
                print(f"✅ [{res['dataset']}] Sup:{res['min_sup']}% | Time: {res['duration']:.2f}s")
            elif res['status'] == 'error':
                print(f"❌ [{res['dataset']}] Error: {res['msg']}")
            else:
                print(f"⚠️  [{res.get('dataset', 'Unknown')}] Exception: {res.get('msg')}")

    print("\n🎉 All experiments finished.")

if __name__ == "__main__":
    main()