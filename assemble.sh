echo "beginning compilation process"
export tundra_VERSION="2025b"
cd tundrac/ && bash build.sh
cd ../mercury/ && bash build.sh
cd ..
cd ./std && ../mercury/mercury build tundrac-path "../tundrac/tundrac"
cd ..
rm -rf ./Release
mkdir ./Release
mkdir ./Release/$tundra_VERSION/
mv ./mercury/mercury ./Release/$tundra_VERSION/
mv ./tundrac/tundrac ./Release/$tundra_VERSION/
cp -r ./std/build/std0.1.0/native ./Release/$tundra_VERSION/std
cd ./Release/$tundra_VERSION/
tar cvzf tundra-$tundra_VERSION-$1.tar.gz .
echo "ended compilation process"