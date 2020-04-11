#include "json.hpp"
#include <string>
#include <unordered_map>
#include <random>
#include <numeric>
#include <algorithm>
#include <optional>
#include <limits>

using json = nlohmann::json;

std::random_device dev;
std::mt19937 engine(dev());
unsigned blockSize;
std::unordered_map<std::string, unsigned> entryPoints;
std::unordered_map<std::string, std::unordered_map<char, unsigned>> characterLinks;

void loadJSON(const std::string& corpus)
{
  auto j = json::parse(corpus);
  blockSize = j["blockSize"];

  for (auto& [key, value] : j["entryPoints"].items())
    entryPoints.emplace(key, value);

  for (auto& [key, value] : j["chain"].items())
  {
    std::unordered_map<char, unsigned> freqs;
    for (auto& [ikey, ivalue] : value.items())
      freqs.emplace(ikey.at(0), ivalue);
    characterLinks.emplace(key, freqs);
  }
}

template<typename T>
std::optional<T> pickRandom(std::mt19937& engine, const std::unordered_map<T, unsigned>& map)
{
  unsigned sum = std::accumulate(map.begin(), map.end(), 0, [](auto value, const auto& p) { return value + p.second; });
  std::uniform_int_distribution<> distribution(0, sum);
  unsigned sel = distribution(engine);
  
  for (auto& [key, value] : map)
  {
    if (sel <= value)
      return key;
    sel -= value;
  }

  return std::nullopt;
}

std::string generate(unsigned length)
{
  std::string word = pickRandom(engine, entryPoints).value();

  unsigned iters = std::max(length, blockSize) - blockSize;
  for (unsigned i = 0; i < iters; ++i)
  {
    std::string key = word.substr(i, blockSize);
    
    if (characterLinks.find(key) == characterLinks.end())
      break;
    
    auto newLetter = pickRandom(engine, characterLinks[key]);
    if (newLetter)
      word += newLetter.value();
  }

  return word;
}

#ifndef STANDALONE
#include <emscripten/bind.h>
using namespace emscripten;

EMSCRIPTEN_BINDINGS(markov)
{
  function("loadJSON", &loadJSON);
  function("generate", &generate);
}
#endif

#ifdef STANDALONE
#include <iostream>
#include <fstream>
#include <streambuf>
int main()
{
  std::ifstream t("test.json");
  std::string str;

  t.seekg(0, std::ios::end);   
  str.reserve(t.tellg());
  t.seekg(0, std::ios::beg);

  str.assign((std::istreambuf_iterator<char>(t)),
             std::istreambuf_iterator<char>());

  loadJSON(str);

  for(;;)
  std::cout << generate(10) << std::endl;
  return 0;
}

#endif
