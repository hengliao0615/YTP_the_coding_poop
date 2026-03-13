import subprocess
import os
import re

# 策略列表包含核心 SCAN、LOOK 與優化後的 Better 策略
STRATEGIES = ["core", "look", "better"]
ITERATIONS = 30      
TICKS = 300

def run_simulation(strategy, ticks):
    print(f"Running {strategy}...") 
    if os.path.exists("events.txt"): os.remove("events.txt")
    if os.path.exists("history.txt"): os.remove("history.txt")
    
    # 執行控制腳本
    subprocess.run(["./controller.sh", strategy, str(ticks)], stdout=subprocess.DEVNULL)
    
    res = {"wait": 0.0, "std": 0.0, "served": 0, "remained": 0}
    if os.path.exists("stats.txt"):
        with open("stats.txt", "r", encoding="utf-8") as f:
            content = f.read()
            # 根據提供的 stats.txt 格式進行正則匹配 
            c_match = re.search(r"實際載運量 \(已送達\): (\d+)", content)
            r_match = re.search(r"滯留乘客人數: (\d+)", content)
            w_match = re.search(r"平均等待時間: ([\d\.]+)", content)
            s_match = re.search(r"等待時間標準差: ([\d\.]+)", content)
            
            if c_match: res["served"] = int(c_match.group(1))
            if r_match: res["remained"] = int(r_match.group(1))
            if w_match: res["wait"] = float(w_match.group(1))
            if s_match: res["std"] = float(s_match.group(1))
    return res

def main():
    # 初始化統計字典，新增 remained 欄位
    final = {s: {"wait": [], "std": [], "served": [], "remained": []} for s in STRATEGIES}
    
    for i in range(ITERATIONS):
        print(f"Iteration {i+1}/{ITERATIONS}")
        for s in STRATEGIES:
            data = run_simulation(s, TICKS)
            final[s]["wait"].append(data["wait"])
            final[s]["std"].append(data["std"])
            final[s]["served"].append(data["served"])
            final[s]["remained"].append(data["remained"])

    # 輸出統計結果表格
    print("\n" + "="*65)
    print(f"{'策略':<10} | {'平均載客':<8} | {'平均滯留':<8} | {'平均等待':<10} | {'標準差':<8}")
    print("-" * 65)
    for s in STRATEGIES:
        avg_w = sum(final[s]["wait"]) / ITERATIONS
        avg_std = sum(final[s]["std"]) / ITERATIONS
        avg_s = sum(final[s]["served"]) / ITERATIONS
        avg_r = sum(final[s]["remained"]) / ITERATIONS
        print(f"{s:<10} | {avg_s:<8.1f} | {avg_r:<8.1f} | {avg_w:<10.2f} | {avg_std:<8.2f}")
    print("="*65)

if __name__ == "__main__":
    main()