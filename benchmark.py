import subprocess
import os
import re

# --- 實驗設定 ---
STRATEGIES = ["core", "better"]
ITERATIONS = 10      # 每個策略跑幾次實驗
TICKS_PER_RUN = 1000 # 每次模擬跑幾步

def run_simulation(strategy, ticks):
    """執行 controller.sh 並回傳平均服務耗時"""
    print(f"正在執行 {strategy} 策略 (模擬 {ticks} 步)...")
    
    # 執行前清理舊數據，確保實驗獨立
    if os.path.exists("events.txt"): os.remove("events.txt")
    if os.path.exists("history.txt"): os.remove("history.txt")
    
    # 調用你的 shell 腳本
    subprocess.run(["./controller.sh", strategy, str(ticks)], 
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    
    # 從 stats.txt 讀取平均數據
    avg_wait = 0.0
    total_served = 0
    if os.path.exists("stats.txt"):
        with open("stats.txt", "r", encoding="utf-8") as f:
            content = f.read()
            # 使用正規表達式抓取數據
            served_match = re.search(r"已完成載客總數: (\d+)", content)
            wait_match = re.search(r"平均服務總耗時: ([\d\.]+)", content)
            
            if served_match: total_served = int(served_match.group(1))
            if wait_match: avg_wait = float(wait_match.group(1))
            
    return total_served, avg_wait

def main():
    results = {s: {"served": [], "wait": []} for s in STRATEGIES}

    for i in range(ITERATIONS):
        print(f"\n--- 第 {i+1} 輪測試 ---")
        for strategy in STRATEGIES:
            served, avg_wait = run_simulation(strategy, TICKS_PER_RUN)
            results[strategy]["served"].append(served)
            results[strategy]["wait"].append(avg_wait)

    # --- 輸出總結報表 ---
    print("\n" + "="*40)
    print(f" 實驗總結 (各執行 {ITERATIONS} 次, 每輪 {TICKS_PER_RUN} 步)")
    print("="*40)
    print(f"{'策略':<15} | {'平均載客人數':<12} | {'平均服務耗時':<12}")
    print("-"*40)

    for s in STRATEGIES:
        final_avg_served = sum(results[s]["served"]) / ITERATIONS
        final_avg_wait = sum(results[s]["wait"]) / ITERATIONS
        print(f"{s:<15} | {final_avg_served:<12.1f} | {final_avg_wait:<12.2f} Ticks")
    
    # 計算進步幅度
    if final_avg_wait > 0:
        improvement = (results["core"]["wait"][0] - results["better"]["wait"][0]) / results["core"]["wait"][0] * 100
        # 這裡簡化計算最後一輪的對比，實際應使用總平均
        core_avg = sum(results["core"]["wait"]) / ITERATIONS
        better_avg = sum(results["better"]["wait"]) / ITERATIONS
        diff = ((core_avg - better_avg) / core_avg) * 100
        print("-"*40)
        print(f"BetterStrategy 效能提升了: {diff:.2f}%")

if __name__ == "__main__":
    main()