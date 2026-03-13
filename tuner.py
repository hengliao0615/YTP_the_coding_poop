import subprocess
import os
import random
import math
import re

# --- 模擬退火參數 ---
ITERATIONS_PER_EVAL = 3   # 每組參數跑幾次模擬取平均，減少隨機噪音
SIM_TICKS = 1000          # 每次模擬步數
INIT_TEMP = 50.0          # 初始溫度
COOLING_RATE = 0.92       # 冷卻率 (建議 0.90 ~ 0.98)
MIN_TEMP = 1.0            # 終止溫度

# 初始參數 [ALPHA, BETA, GAMMA, C1, C2]
current_p = [10.0, 1.5, 2.0, 2.0, 0.1]

def get_simulation_cost(params):
    """將參數寫入並執行模擬，回傳加權 Cost"""
    with open("params.txt", "w") as f:
        f.write(" ".join(map(str, params)))
    
    total_wait, total_std, total_max = 0, 0, 0
    
    for _ in range(ITERATIONS_PER_EVAL):
        if os.path.exists("events.txt"): os.remove("events.txt")
        # 執行 Better Strategy
        subprocess.run(["./controller.sh", "better", str(SIM_TICKS)], 
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        
        if os.path.exists("stats.txt"):
            with open("stats.txt", "r") as f:
                content = f.read()
                w = re.search(r"平均等待時間: ([\d\.]+)", content)
                s = re.search(r"等待時間標準差: ([\d\.]+)", content)
                m = re.search(r"最大等待時間: (\d+)", content)
                if w: total_wait += float(w.group(1))
                if s: total_std += float(s.group(1))
                if m: total_max += int(m.group(1))
    
    avg_wait = total_wait / ITERATIONS_PER_EVAL
    avg_std = total_std / ITERATIONS_PER_EVAL
    avg_max = total_max / ITERATIONS_PER_EVAL
    
    # Cost = 平均等待時間 + 標準差加成 + 極端值懲罰
    # 你可以根據你的偏好調整這些加權係數 (如 0.5 或 0.1)
    return avg_wait + (avg_std * 0.4) + (avg_max * 0.1)

def main():
    print("--- Start Simulated Annealing Optimization ---")
    subprocess.run(["make", "all"], stdout=subprocess.DEVNULL)
    
    temp = INIT_TEMP
    curr_cost = get_simulation_cost(current_p)
    best_p = list(current_p)
    best_cost = curr_cost
    
    print(f"Initial Avg Cost: {curr_cost:.2f}")

    while temp > MIN_TEMP:
        # 產生新鄰居：隨機微調參數，幅度隨溫度降低而縮小
        new_p = [max(0.01, p + random.uniform(-1.5, 1.5) * (temp/INIT_TEMP)) for p in current_p]
        new_cost = get_simulation_cost(new_p)
        
        delta = new_cost - curr_cost
        # Metropolis 準則：更優則接受；較差則以機率接受以跳出局部最佳
        if delta < 0 or random.random() < math.exp(-delta / temp):
            current_p[:] = new_p
            curr_cost = new_cost
            if curr_cost < best_cost:
                best_cost = curr_cost
                best_p = list(new_p)
                print(f"[*] Found Better! Cost: {best_cost:.2f} | Params: {[round(x,2) for x in best_p]}")
        
        temp *= COOLING_RATE
        print(f"Temp: {temp:.2f} | Current Cost: {curr_cost:.2f}")

    print("\n" + "="*30)
    print(f"Optimal Params: ALPHA={best_p[0]:.2f}, BETA={best_p[1]:.2f}, GAMMA={best_p[2]:.2f}, C1={best_p[3]:.2f}, C2={best_p[4]:.2f}")
    print(f"Best Cost: {best_cost:.2f}")
    
    # 最後將最優參數存入 params.txt 供後續模擬使用
    with open("params.txt", "w") as f:
        f.write(" ".join(map(str, best_p)))

if __name__ == "__main__":
    main()