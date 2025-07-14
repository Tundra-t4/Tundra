cd ./modules/HParser && bash build.sh
cd ../../
cd ../polaroid && bash build.sh
cd ../tundrac && bash ctundrac.sh
./tundrac compile simple.ta t4path "/Users/dusk/Documents/Github/maven/std/build/std0.1.0/native/" target-cpu apple-m3 emit llvm-ir