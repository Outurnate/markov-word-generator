#!/bin/bash
pushd "${0%/*}"
pushd corpus_generator
cargo build --release
popd
mkdir -p .build
pushd .build
wget https://github.com/dwyl/english-words/raw/master/words.txt -O dwyl-word-list.txt
wget https://github.com/first20hours/google-10000-english/raw/master/google-10000-english-no-swears.txt -O google-10000-english-no-swears.txt
wget https://github.com/dominictarr/random-name/raw/master/first-names.txt -O dominictarr-first-names.txt
wget https://github.com/arineng/arincli/raw/master/lib/last-names.txt -O arineng-last-names.txt
git clone https://github.com/emscripten-core/emsdk.git
pushd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
popd
wget https://github.com/nlohmann/json/releases/download/v3.7.3/json.hpp
popd
./corpus_generator/target/release/corpus_generator .build/dwyl-word-list.txt > www/dwyl-word-list.json
./corpus_generator/target/release/corpus_generator .build/google-10000-english-no-swears.txt > www/google-10000-english-no-swears.json
./corpus_generator/target/release/corpus_generator .build/dominictarr-first-names.txt > www/dominictarr-first-names.json
./corpus_generator/target/release/corpus_generator .build/arineng-last-names.txt > www/arineng-last-names.json
em++ -O2 --llvm-opts 2 -flto -std=c++1z --bind src/markov.cpp -I.build -s WASM=1 -o www/markov.js
popd
