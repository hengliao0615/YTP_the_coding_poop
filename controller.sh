#!/bin/bash

# ==========================================================
# 電梯調度專題 模擬控制腳本 (controller.sh)
# 使用方式: 
#   ./controller.sh better 1000  (執行新策略)
#   ./controller.sh core 500     (執行 SCAN 策略)
#   ./controller.sh look 500     (執行 LOOK 策略)
# ==========================================================

# 參數 1: 策略選擇 (預設為 better)
STRATEGY_TYPE=${1:-better}
# 參數 2: 模擬總步數 (預設為 1000)
MAX_STEPS=${2:-1000}
JSON_FILE="log.json"

# 1. 自動偵測並編譯所有模組
echo "--- 正在編譯專案模組 ---"
make all || { echo "編譯失敗！請檢查原始碼。"; exit 1; }

# 2. 從 config.txt 讀取大樓設定
if [ ! -f config.txt ]; then
    echo "錯誤: 找不到 config.txt，請先建立設定檔。"
    exit 1
fi

F_COUNT=$(awk 'NR==1 {print $1}' config.txt)
E_CAP=$(awk 'NR==1 {print $3}' config.txt)
E_COUNT=$(awk 'NR==1 {print $4}' config.txt)

# 3. 根據輸入參數決定執行路徑 (新增 look 判斷)
if [ "$STRATEGY_TYPE" == "better" ]; then
    STR_EXE="./BetterStrategy/better_strategy"
    STR_NAME="BetterStrategy (Score-Based)"
elif [ "$STRATEGY_TYPE" == "core" ]; then
    STR_EXE="./CoreStrategy/core_strategy"
    STR_NAME="CoreStrategy (SCAN)"
elif [ "$STRATEGY_TYPE" == "look" ]; then
    STR_EXE="./LookStrategy/look_strategy"
    STR_NAME="LookStrategy (LOOK)"
else
    echo "錯誤: 不支援的策略類型 '$STRATEGY_TYPE'。請使用 'core', 'look' 或 'better'。"
    exit 1
fi

# 4. 初始化環境與狀態檔案
echo -n "" > state.txt
for (( i=0; i<$E_COUNT; i++ )); do
    echo "0 0 0 0" >> state.txt
done

echo "0" > id_counter.txt
> passengers.txt
> events.txt
> stats.txt

# 5. 準備 JSON 檔頭
FLOOR_NAMES_JSON=""
for (( i=1; i<=$F_COUNT; i++ )); do
    FLOOR_NAMES_JSON+="\"$i\""
    [ $i -lt $F_COUNT ] && FLOOR_NAMES_JSON+=", "
done

cat <<EOF > $JSON_FILE
{
  "metadata": {
    "version": "2.0",
    "totalTicks": $MAX_STEPS,
    "strategy": "$STR_NAME"
  },
  "config": {
    "floorCount": $F_COUNT,
    "elevatorCount": $E_COUNT,
    "elevatorCapacity": $E_CAP,
    "floorNames": [$FLOOR_NAMES_JSON]
  },
  "timeline": [
EOF

echo "--- 模擬開始: $STR_NAME ---"
echo "大樓規模: $F_COUNT 樓 | 電梯數量: $E_COUNT | 總步數: $MAX_STEPS"

# 6. 主模擬循環
for (( i=0; i<$MAX_STEPS; i++ ))
do
    > tmp_event.json
    
    ./PassengerGenerator/generator $i || { echo "Generator 在 Tick $i 崩潰"; exit 1; }
    $STR_EXE $i || { echo "Strategy 在 Tick $i 崩潰"; exit 1; }
    
    # 注意：原本的腳本在循環內呼叫 stats 會逐秒更新 stats.txt 
    # 若需計算「滯留乘客」，stats 應能在最後一秒接收 $MAX_STEPS 參數
    ./StatisticsTracker/stats $i || { echo "Stats 在 Tick $i 崩潰"; exit 1; }
    
    if [ -s tmp_event.json ]; then
        echo "    {" >> $JSON_FILE
        echo "      \"tick\": $i," >> $JSON_FILE
        echo "      \"events\": [" >> $JSON_FILE
        sed '$ s/,$//' tmp_event.json >> $JSON_FILE
        echo "      ]" >> $JSON_FILE
        
        if [ $i -lt $((MAX_STEPS-1)) ]; then
            echo "    }," >> $JSON_FILE
        else
            echo "    }" >> $JSON_FILE
        fi
    fi
    
    if (( i % 20 == 0 )); then
        printf "\r模擬進度: [%-50s] %d/%d" $(printf "#%.0s" $(seq 1 $((i * 50 / MAX_STEPS)))) "$i" "$MAX_STEPS"
    fi
done

# 7. 閉合 JSON 檔結尾
sed -i '$ s/},/}/' $JSON_FILE
echo "  ]" >> $JSON_FILE
echo "}" >> $JSON_FILE

rm -f tmp_event.json

# 額外統計輸出 (直接讀取 stats.txt 內容顯示)
echo -e "\n\n--- 模擬結束 ---"
echo "完整紀錄: $JSON_FILE"
echo "數據統計 (stats.txt):"
cat stats.txt