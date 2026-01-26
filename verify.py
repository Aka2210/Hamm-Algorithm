import sys

str_to_id = {}
id_to_str = []

def get_id(s):
    if s not in str_to_id:
        new_id = len(id_to_str)
        str_to_id[s] = new_id
        id_to_str.append(s)
        return new_id
    return str_to_id[s]

def clean_string(s):
    return s.strip(" \t\r\n")

def load_data(filepath):
    transactions = []
    print(f"📂 Reading Raw Data (Building ID Map)...")
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            line = line.rstrip('\r\n ')
            if not line: continue
            
            raw_parts = line.split(',')
            
            line_ids = set()
            for p in raw_parts:
                clean_p = clean_string(p)
                if not clean_p: continue
                
                idx = get_id(clean_p)
                line_ids.add(idx)
            
            transactions.append(line_ids)
    return transactions

def load_results(filepath):
    patterns = []
    print(f"📂 Reading Results (Mapping back to IDs)...")
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            line = line.strip()
            if not line: continue
            try:
                if ':' not in line: continue
                content, sup_str = line.split(':')
                
                if not content:
                    item_ids = set()
                else:
                    parts = content.split(',')
                    item_ids = set()
                    for p in parts:
                        p = p.strip()
                        if p in str_to_id:
                            item_ids.add(str_to_id[p])
                        else:
                            print(f"⚠️ Warning: Result contains unknown string '{p}'")
                            item_ids.add(get_id(p))
                
                support_ratio = float(sup_str)
                patterns.append((item_ids, support_ratio))
            except ValueError:
                continue
    return patterns

def verify(data_path, result_path, tol=1e-4):
    print(f"🔍 Mode: Strict ID Matching (Same as C++ Logic)")

    transactions = load_data(data_path)
    total_tx = len(transactions)
    print(f"   Loaded {total_tx} transactions.")
    print(f"   Total Unique Items (IDs): {len(str_to_id)}")

    patterns = load_results(result_path)
    print(f"   Loaded {len(patterns)} patterns to verify.")

    print("-" * 50)
    print("Start Verifying...")
    
    error_count = 0
    
    for items, reported_sup in patterns:
        actual_count = 0
        for tx in transactions:
            if items.issubset(tx):
                actual_count += 1
        
        actual_sup = actual_count / total_tx
        
        if abs(actual_sup - reported_sup) > tol:
            error_count += 1
            readable_items = {id_to_str[i] for i in items}
            print(f"❌ Mismatch! Pattern: {readable_items}")
            print(f"   C++ Reported: {reported_sup:.4f}")
            print(f"   Python Check: {actual_sup:.4f} ({actual_count}/{total_tx})")
            
            if error_count > 10:
                print("... Too many errors, stopping.")
                break

    print("-" * 50)
    if error_count == 0:
        print(f"✅ PASSED! All {len(patterns)} patterns are correct.")
    else:
        print(f"❌ FAILED! Found {error_count} errors out of {len(patterns)}.")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 verify.py <raw_data_path> <result_file_path>")
        sys.exit(1)
    
    verify(sys.argv[1], sys.argv[2])