import subprocess
import os
import re

STRATEGIES = ["core", "better"]
ITERATIONS = 5      
TICKS = 100

def run_simulation(strategy, ticks):
    print(f"Running {strategy}...")
    if os.path.exists("events.txt"): os.remove("events.txt")
    if os.path.exists("history.txt"): os.remove("history.txt")
    
    subprocess.run(["./controller.sh", strategy, str(ticks)], stdout=subprocess.DEVNULL)
    
    res = {"wait": 0.0, "std": 0.0, "served": 0}
    if os.path.exists("stats.txt"):
        with open("stats.txt", "r") as f:
            content = f.read()
            c_match = re.search(r"已完成載客總數: (\d+)", content)
            w_match = re.search(r"平均等待時間: ([\d\.]+)", content)
            s_match = re.search(r"等待時間標準差: ([\d\.]+)", content)
            if c_match: res["served"] = int(c_match.group(1))
            if w_match: res["wait"] = float(w_match.group(1))
            if s_match: res["std"] = float(s_match.group(1))
    return res

def main():
    final = {s: {"wait": [], "std": [], "served": []} for s in STRATEGIES}
    for i in range(ITERATIONS):
        for s in STRATEGIES:
            data = run_simulation(s, TICKS)
            final[s]["wait"].append(data["wait"])
            final[s]["std"].append(data["std"])
            final[s]["served"].append(data["served"])

    print("\n" + "="*50)
    print(f"{'策略':<10} | {'平均載客':<8} | {'平均等待':<10} | {'標準差':<8}")
    print("-" * 50)
    for s in STRATEGIES:
        avg_w = sum(final[s]["wait"]) / ITERATIONS
        avg_std = sum(final[s]["std"]) / ITERATIONS
        avg_s = sum(final[s]["served"]) / ITERATIONS
        print(f"{s:<10} | {avg_s:<8.1f} | {avg_w:<10.2f} | {avg_std:<8.2f}")

if __name__ == "__main__":
    main()