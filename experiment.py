import argparse
import os
import subprocess
import concurrent.futures
import math

MAX_WORKERS = 4 

def run_hamm(dataset_name, input_file, min_sup_rate, output_file):
    cmd = [
        "./tools/hamm",
        str(min_sup_rate),
        input_file,
        output_file
    ]
    result = subprocess.run(cmd, capture_output=True, text=True)
    return result

def preprocess_data(dataset_path, output_path, ratio=None, size=None):
    with open(dataset_path, 'r') as f:
        lines = f.readlines()
    
    total_lines = len(lines)
    keep_lines = total_lines
    
    if size is not None:
        keep_lines = int(size)
    elif ratio is not None:
        keep_lines = int(total_lines * (float(ratio) / 100.0))
        
    if keep_lines > total_lines:
        print(f"Warning: Requested size {keep_lines} > total size {total_lines}. Using full dataset.")
        keep_lines = total_lines

    with open(output_path, 'w') as f:
        f.writelines(lines[:keep_lines])
    
    return output_path

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--datasets", type=str, required=True, help="comma separated dataset names")
    parser.add_argument("--tx-ratios", type=str, help="comma separated ratios e.g. 10,20")
    parser.add_argument("--minsup-ratios", type=str, required=True, help="comma separated minsup ratios e.g. 0.5,1,2")
    parser.add_argument("--tx-size", type=int, help="Specific number of transactions (overrides tx-ratios if set)")
    parser.add_argument("--override-default-minsup", type=str, default="", help="dataset specific base minsups")
    parser.add_argument("--resume", action="store_true", help="Skip existing results")
    
    args = parser.parse_args()

    dataset_list = args.datasets.split(',')
    minsup_list = [float(x) for x in args.minsup_ratios.split(',')]
    
    base_minsups = {}
    if args.override_default_minsup:
        for item in args.override_default_minsup.split(','):
            k, v = item.split('=')
            base_minsups[k.strip()] = float(v)

    tasks = []

    for ds in dataset_list:
        raw_path = f"data_raw/{ds}.data"
        if not os.path.exists(raw_path):
            print(f"Skipping {ds}, file not found.")
            continue

        slices = []
        if args.tx_size:
            slices.append(('size', args.tx_size))
        elif args.tx_ratios:
            for r in args.tx_ratios.split(','):
                slices.append(('ratio', float(r)))
        else:
            slices.append(('ratio', 100))

        for slice_type, slice_val in slices:
            slice_name = f"{slice_val}"
            temp_input = f"data_raw/temp_{ds}_{slice_type}_{slice_name}.txt"
            
            if slice_type == 'size':
                preprocess_data(raw_path, temp_input, size=slice_val)
            else:
                preprocess_data(raw_path, temp_input, ratio=slice_val)

            base_ms = base_minsups.get(ds, 1.0) 
            
            for ms_ratio in minsup_list:
                final_rate = (base_ms * ms_ratio) / 100.0 
                
                out_file = f"results/{ds}_{slice_type}{slice_name}_sup{ms_ratio}.txt"
                
                if args.resume and os.path.exists(out_file):
                    print(f"Skipping {out_file}, already exists.")
                    continue

                tasks.append((ds, temp_input, final_rate, out_file))

    print(f"Total tasks: {len(tasks)}")
    
    with concurrent.futures.ThreadPoolExecutor(max_workers=MAX_WORKERS) as executor:
        futures = []
        for task in tasks:
            futures.append(executor.submit(run_hamm, *task))
        
        for future in concurrent.futures.as_completed(futures):
            pass

    print("All experiments finished.")

if __name__ == "__main__":
    main()