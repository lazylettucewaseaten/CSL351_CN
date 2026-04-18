#!/bin/bash

mkdir -p as8_results
output_file="as8_results/jains_index_table.md"

echo "running the codes "
./ns3 build

{
    echo "| Flows                | 4      | 8      | 16     | 20     |"
} | tee "$output_file"

printf "| NewReno (Baseline)   |" | tee -a "$output_file"
for n in 4 8 16 20; do
    res=$(./ns3 run "as8_que1 --nFlows=$n --isMixed=false" 2>/dev/null | tail -n 1)
    printf " %-6.4f |" "$res" | tee -a "$output_file"
done
echo "" | tee -a "$output_file"

printf "| NewReno & TcpAshish  |" | tee -a "$output_file"
for n in 4 8 16 20; do
    res=$(./ns3 run "as8_que1 --nFlows=$n --isMixed=true" 2>/dev/null | tail -n 1)
    printf " %-6.4f |" "$res" | tee -a "$output_file"
done
echo "" | tee -a "$output_file"

mv Baseline_*.txt as8_results/ 2>/dev/null
mv Mixed_*.txt as8_results/ 2>/dev/null
mv *.pcap as8_results/ 2>/dev/null