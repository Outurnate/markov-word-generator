use std::fs::File;
use std::io::{BufRead, BufReader};
use regex::Regex;
use std::collections::HashMap;
use serde::{Deserialize, Serialize};
use serde_json::Result;
use std::env;

const blockSize: usize = 3;

#[derive(Serialize, Deserialize)]
struct Corpus
{
  chain: HashMap<String, HashMap<char, usize>>,
  entryPoints: HashMap<String, usize>,
  blockSize: usize
}

fn main()
{
  let args: Vec<String> = std::env::args().collect();
  let filename = &args[1];
  let file = File::open(filename).unwrap();
  let reader = BufReader::new(file);
  let filter = Regex::new(r"^[a-z]{4,}$").unwrap();

  let mut charLinks = HashMap::new();
  let mut entryPoints = HashMap::new();

  for (index, line) in reader.lines().enumerate()
  {
    let line = line.unwrap().to_lowercase();
    if line.len() >= blockSize && filter.is_match(&line)
    {
      let charVec: Vec<char> = line.chars().collect();

      // get starting characters
      let entry = &line[0..blockSize].to_string();
      if !entryPoints.contains_key(entry)
      {
        entryPoints.insert(entry.to_owned(), 1);
      }
      else
      {
        entryPoints.insert(entry.to_owned(), entryPoints.get(entry).unwrap() + 1);
      }
      
      // read through word
      for i in 0..(charVec.len() - blockSize - 1)
      {
        let key = &line[i..(i + blockSize)].to_string();
        let nextChar = charVec[i + blockSize];
        if !charLinks.contains_key(key)
        {
          charLinks.insert(key.to_owned(), HashMap::new());
        }
        let mut link = charLinks.get(key).unwrap().to_owned();
        if !link.contains_key(&nextChar)
        {
          link.insert(nextChar, 1);
        }
        else
        {
          link.insert(nextChar, link.get(&nextChar).unwrap() + 1);
        }
        charLinks.insert(key.to_owned(), link);
      }
    }
  }

  let j = serde_json::to_string(&Corpus { chain: charLinks, entryPoints: entryPoints, blockSize: blockSize });
  println!("{}", j.unwrap());
}
