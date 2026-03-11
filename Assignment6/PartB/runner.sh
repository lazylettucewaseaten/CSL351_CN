#!/bin/bash
#ROLL NO 12340370 so last two digit as 70 , hardcoded 70 in .cc in case not specified from the temrinal
BASE_SEED=70
MAX_SEED=$((BASE_SEED + 9)) 

ERR_RATES=("0.0" "0.2" "0.4" "0.6" "0.8" "1.0")
ERR_UNITS=("Packet" "Bit" "Byte")

if [ ! -x "./ns3" ]; then
    echo "./ns3 not found. Run from the ns3 directory."
    exit 1
fi

for u in "${ERR_UNITS[@]}"; do
    outfile="outputdata${u}.dat"
    echo "# ErrorRate AvgPDR" > "$outfile"
    
    echo "Starting Error Unit: $u"
    
    for r in "${ERR_RATES[@]}"; do
        sum_pdr=0
        
        for s in $(seq $BASE_SEED $MAX_SEED); do
            out=$(./ns3 run "scratch/as6_que2 --errorRate=$r --errorUnit=$u --seedRun=$s" 2>&1)
            
            pdr_val=$(echo "$out" | grep "PDR_RESULT:" | cut -d':' -f2)
            
            if [ -z "$pdr_val" ]; then
                pdr_val=0
            fi
            
            sum_pdr=$(awk "BEGIN {print $sum_pdr + $pdr_val}")
            
            echo "Unit: $u | Rate: $r | Seed: $s | PDR: $pdr_val"
        done
        
        avg_pdr=$(awk "BEGIN {print $sum_pdr / 10}")
        echo "$r $avg_pdr" >> "$outfile"
        echo "Average PDR for $r ($u): $avg_pdr"
        echo "-----------------------------------"
    done
done

echo "All simulations complete. Data saved to .dat files."