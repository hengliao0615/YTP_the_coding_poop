#!/bin/bash

# 參數 1: 策略選擇 (core 或 better)
# 參數 2: 步數 (預設 1000)
STRATEGY_TYPE=${1:-better}
MAX_STEPS=${2:-1000}
JSON_FILE="log.json"

# 設定執行檔路徑
if [ "$STRATEGY_TYPE" == "better" ]; then
    STR_EXE="./BetterStrategy/better_strategy"
    STR_NAME="BetterStrategy"
elif [ "$STRATEGY_TYPE" == "core" ]; then
    STR_EXE="./CoreStrategy/core_strategy"
    STR_NAME="CoreStrategy"
else
    echo "錯誤: 請選擇策略類型 [core|better]"
    exit 1
fi

make all || exit 1

# 1. 從 config.txt 抓取設定
F_COUNT=$(awk 'NR==1 {print $1}' config.txt)
E_CAP=$(awk 'NR==1 {print $3}' config.txt)
E_COUNT=$(awk 'NR==1 {print $4}' config.txt)

# 2. 初始化狀態 (統一初始化 4 個參數以相容 BetterStrategy)
echo -n "" > state.txt
for (( i=0; i<$E_COUNT; i++ )); do
    echo "0 0 0 0" >> state.txt
done

echo "0" > id_counter.txt
> passengers.txt
> events.txt

# 3. 生成 JSON 檔頭
FLOOR_NAMES_JSON=""
for (( i=1; i<=$F_COUNT; i++ )); do
    FLOOR_NAMES_JSON+="\"$i\""
    [ $i -lt $F_COUNT ] && FLOOR_NAMES_JSON+=", "
done

cat <<EOF > $JSON_FILE
{
  "metadata": {
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

echo "--- 啟動模擬: $STR_NAME ---"

# 4. 模擬迴圈
for (( i=0; i<$MAX_STEPS; i++ ))
do
    > tmp_event.json
    ./PassengerGenerator/generator $i
    $STR_EXE $i  # 執行被選取的策略
    ./StatisticsTracker/stats $i
    
    if [ -s tmp_event.json ]; then
        echo "    {" >> $JSON_FILE
        echo "      \"tick\": $i," >> $JSON_FILE
        echo "      \"events\": [" >> $JSON_FILE
        sed '$ s/,$//' tmp_event.json >> $JSON_FILE
        echo "      ]" >> $JSON_FILE
        [ $i -lt $((MAX_STEPS-1)) ] && echo "    }," >> $JSON_FILE || echo "    }" >> $JSON_FILE
    fi
    printf "\rProgress: %d/%d" "$((i+1))" "$MAX_STEPS"
done

sed -i '$ s/},/}/' $JSON_FILE
echo "  ]" >> $JSON_FILE
echo "}" >> $JSON_FILE

rm -f tmp_event.json
echo -e "\n模擬完成！結果儲存於 $JSON_FILE"