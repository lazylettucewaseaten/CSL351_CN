#!/bin/bash
mkdir -p results

declare -a tcpvar=("TcpNewReno" "TcpHighSpeed" "TcpVeno" "TcpVegas")
declare -a bws=("20Mbps" "50Mbps" "100Mbps") 
declare -a lcs=("20ms" "50ms" "100ms") 

for tcp in "${tcpvar[@]}"; do

    name=$(echo "$tcp" | sed 's/Tcp//' | tr '[:upper:]' '[:lower:]')
    
    echo "testing $tcp"
    echo "  "

    
    echo "[Base setup: 5Mbps, 2ms]"
    ./ns3 run "as7_que2 --tcpVariant=$tcp --linkBw=5Mbps --linkDelay=2ms" > ns3_output.log 2>&1
    
    # saveig plots acc to the name defines inthe ques
    mv seventh.cwnd results/${name}_cwnd.txt
    mv seventh_rtt.txt results/${name}_rtt.txt
    mv seventh-ssthresh.txt results/${name}_ssthresh.txt
    mv seventh-file-0-0.pcap results/${name}_node0.pcap
    mv seventh-file-1-0.pcap results/${name}_node1.pcap
    
    tput=$(capinfos -i results/${name}_node1.pcap | grep "Data bit rate" | awk -F: '{print $2}' | xargs)
    echo "   Avg throughput: $tput"
    
    drops=$(awk '$3 < $2 {c++} END {print c+0}' results/${name}_cwnd.txt)
    echo "   Cwnd dropped $drops times "
    
    echo -e "\n testing changing Bandwidth  (delay fixed at 2ms)"
    for bw in "${bws[@]}"; do
        ./ns3 run "as7_que2 --tcpVariant=$tcp --linkBw=$bw --linkDelay=2ms" >> ns3_output.log 2>&1
        
        tmp_pcap="results/${name}_${bw}_2ms_node1.pcap"
        mv seventh-file-1-0.pcap $tmp_pcap
        rm -f seventh-file-0-0.pcap seventh.cwnd seventh_rtt.txt seventh-ssthresh.txt
        
        b_tput=$(capinfos -i $tmp_pcap | grep "Data bit rate" | awk -F: '{print $2}' | xargs)
        echo "   BW: $bw , Throughput: $b_tput"
    done

    echo -e "\n testing changing latency (BW fixed at 5Mbps)"
    for del in "${lcs[@]}"; do
        ./ns3 run "as7_que2 --tcpVariant=$tcp --linkBw=5Mbps --linkDelay=$del" >> ns3_output.log 2>&1
        
        tmp_pcap="results/${name}_5Mbps_${del}_node1.pcap"
        mv seventh-file-1-0.pcap $tmp_pcap
        rm -f seventh-file-0-0.pcap seventh.cwnd seventh_rtt.txt seventh-ssthresh.txt
        
        d_tput=$(capinfos -i $tmp_pcap | grep "Data bit rate" | awk -F: '{print $2}' | xargs)
        echo "   Delay: $del  , Throughput: $d_tput"
    done
    echo ""
done
