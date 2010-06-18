# !/bin/sh
# Add you want to run daemon
# cd root
# ./cfg_bts2_nps7_sa_1.sh
sed 's/.$//' moxa7gate.sh > moxa.sh
chmod +x moxa.sh
./moxa.sh