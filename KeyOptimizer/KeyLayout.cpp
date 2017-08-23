#include "stdafx.h"
#include "KeyLayout.h"
#include <cstdlib>
#include <assert.h>
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <algorithm>
#include <chrono>
#include "PolygramFrequency.h"

using namespace std;

static inline unsigned char getIndex(const char c) {
  if (c < 0 || c >= 128)
    return 0;
  return c;
}
static inline unsigned char getChar(const char i) {
  return i;
}

KeyLayout::KeyLayout(PolygramFrequency& frequency) : freq{ frequency } {
  int char_set_size = freq.char_set_size;
  int max_length_polygram = freq.getMaxLengthPolygram();
  assert(max_length_polygram >= 2);
  mapping.resize(char_set_size);
  m.resize(char_set_size);
  for (int i = 0; i < mapping.size(); ++i) {
    mapping[i].resize(char_set_size);
    m[i].resize(char_set_size);
  }
  resetKeymapping();
}
void KeyLayout::resetKeymapping() {
  const int char_set_size = PolygramFrequency::char_set_size;
  for (int i = 0; i < mapping.size(); ++i)
    for (int j = 0; j < mapping[i].size(); ++j) {
      mapping[i][j] = 0;
      m[i][j] = 0;
    }
  visual_rep = "";
  character_groups.clear();
  should_move_character.clear();
  current_score = 0;
}

void KeyLayout::addInputWeights(const string start, const string dest, const float effort) {
  for (int i = 0; i < start.length(); i++)
    for (int j = 0; j < dest.length(); j++)
      mapping[getIndex(start[i])][getIndex(dest[j])] += effort;
}
void KeyLayout::minInputWeights(const string start, const string dest, const float effort) {
  for (int i = 0; i < start.length(); i++)
    for (int j = 0; j < dest.length(); j++)
      mapping[getIndex(start[i])][getIndex(dest[j])] = max(effort, mapping[getIndex(start[i])][getIndex(dest[j])]);
}
void KeyLayout::maxInputWeights(const string start, const string dest, const float effort) {
  for (int i = 0; i < start.length(); i++)
    for (int j = 0; j < dest.length(); j++)
      mapping[getIndex(start[i])][getIndex(dest[j])] += min(effort, mapping[getIndex(start[i])][getIndex(dest[j])]);
}
const int group_offset = 2;
void KeyLayout::addNDimGroups(vector<string> vec) {
  int vec_offset_size = vec.size() - group_offset;
  //fill in any blank spaces below vec_offset_size in character_groups
  while (character_groups.size() < vec_offset_size) {
    vector<string> blank;
    for (int i = 2; i < character_groups.size(); i++)
      blank.push_back("");
    character_groups.push_back(blank);
  }
  //if this is the first time then save time by just indexing the parameter vec
  if (character_groups.size() == vec_offset_size)
    character_groups.push_back(vec);
  else
  //copy the parameter vec into the currently indexed vec
  {
    vector<string> old_vec = character_groups[vec_offset_size];
    assert(vec.size() == old_vec.size());
    for (int i = 0; i < vec.size(); i++)
      old_vec[i] += vec[i];
  }  
}
void KeyLayout::setMustMoveInNDimGroups(const int n, string characters) {
  int n_offset_size = n - group_offset;
  //fill in any blank spaces below n_offset_size in should_move_character
  while (should_move_character.size() < n)
    should_move_character.push_back("");
  //if this is the first time then save time by just indexing the parameter vec
  if (should_move_character.size() == n_offset_size)
    should_move_character.push_back(characters);
  else
  //copy the paramter vec into the currently indexed vec
  {
    should_move_character[n_offset_size] += characters;
  }  
}
void KeyLayout::setVisualRep(string rep) {
  visual_rep = rep;
}
void KeyLayout::setVisualRep(vector<string> vec) {
  visual_rep = "\1";
  for (string st : vec) {
    visual_rep += st;
    visual_rep += '\1';
  }
}
string KeyLayout::getVisualRep() {
  return visual_rep;
}

float KeyLayout::getNewMapping(int length, vector<int>& chars) {
  if (length == 2)
    return mapping[chars[0]][chars[1]];
  float max = mapping[chars[0]][chars[length - 1]];
  for (int i = 1; i < length - 1; i++) {
    int val = getMapping(length - i, chars) + mapping[chars[length - 1 - i]][chars[length - 1]];
    if (val > max)
      max = val;
  }
  return max;
}
float KeyLayout::getMapping(int length, vector<int>& chars) {
  if (length == 2)
    return m[chars[0]][chars[1]];
  float max = m[chars[0]][chars[length - 1]];
  for (int i = 1; i < length - 1; i++) {
    int val = getMapping(length - i, chars) + m[chars[length - 1 - i]][chars[length - 1]];
    if (val > max)
      max = val;
  }
  return max;
}
float KeyLayout::forceUpdateScore() {
  const int char_set_size = PolygramFrequency::char_set_size;
  int polygram_level = freq.getMaxLengthPolygram();
  for (int i = 0; i < char_set_size; ++i)
    for (int j = 0; j < char_set_size; ++j)
      m[i][j] = mapping[i][j];
  current_score = 0;
  vector<int> f (polygram_level);
  for (int i = 0; i < char_set_size; i++) {
    f[0] = i;
    for (int j = 0; j < char_set_size; j++) {
      f[1] = j;
      float m2 = getMapping(2, f);
      current_score += m2 * freq.getFrequency(2, f);
      if (polygram_level < 3)
        continue;
      for (int k = 0; k < char_set_size; k++) {
        float m3 = max({ m[i][k], m2 + m[j][k] });
        current_score += m3 * freq.getFrequency(3, f);
        if (polygram_level < 4)
          continue;
        for (int l = 0; l < char_set_size; l++) {
          float m4 = max({ m[i][l], m3 + m[k][l], m[i][j] + m[j][l] });
          current_score += m4 * freq.getFrequency(4, f);
        }
      }
    }
  }
  float ans = current_score / (polygram_level - 1) / freq.getTotalCharacters();
  return ans;
}
float KeyLayout::adaptiveUpdateScore() {
  const int char_set_size = PolygramFrequency::char_set_size;
  int polygram_level = freq.getMaxLengthPolygram();
  vector<int> f (polygram_level);
  while(true){
    float new_mapping = getNewMapping(polygram_level, f);
    float mapping = getMapping(polygram_level, f);
    if(new_mapping != mapping)
      current_score += (new_mapping - mapping) * freq.getFrequency(polygram_level, f);
    bool carry_out = true;
    for (int i = f.size() - 1; i >= 0 && carry_out; --i) {
      carry_out = false;
      do{
        if (f[i] == char_set_size - 1) {
          f[i] = 0;
          carry_out = true;
          break;
        } else {
          ++f[i];
        }
      } while (freq.getFrequency(i + 1, f) == 0);
    }
    if (carry_out)
      break;
  }
  for (int i = 0; i < char_set_size; ++i) {
    if (freq.getFrequency(1, f) == 0)
      continue;
    for (int j = 0; j < char_set_size; ++j)
      if (m[i][j] != mapping[i][j])
        m[i][j] = mapping[i][j];
  }
  float ans = current_score / (polygram_level - 1) / freq.getTotalCharacters();
  return ans;
}
void KeyLayout::swapLetters(string& str, char a, char b) {
  for (int i = 0; i < str.length(); i++) {
    if (str[i] == a)
      str[i] = b;
    else if (str[i] == b)
      str[i] = a;
  }
}
bool KeyLayout::swapCharacters(string a, string b) {
  auto t1 = chrono::system_clock::now();
  const int char_set_size = PolygramFrequency::char_set_size;
  string first = "";
  string second = "";
  //check if is swappable;
  for (int char_index = 0; char_index < a.length(); char_index++) {
    if (first.find_first_of(a[char_index]) != -1 || second.find_first_of(b[char_index]) != -1)
      return false;
    for (int i = should_move_character.size() - 1; i >= 0; i--) {
      //see if one wants to be swapped in the nth dimension
      if (should_move_character[i].find_first_of(a[char_index]) != -1 || should_move_character[i].find_first_of(b[char_index]) != -1) {
        vector<string> group = character_groups[i];
        //get the indexes in the group
        int first_index = -1, second_index = -1;
        for (int j = 0; j < group.size() && first_index == -1; j++) {
          first_index = group[j].find_first_of(a[char_index]);
          second_index = group[j].find_first_of(b[char_index]);
        }
        //if a letter was not found or they are in different sub-groups then they're not swappable
        if (first_index == -1 || second_index == -1)
          return false;
        for (int j = 0; j < group.size(); j++) {
          first += group[j][first_index];
          second += group[j][second_index];
        }
      }
    }
  }
  //swap letters in mapping, visual_rep, character_groups
  bool already_swapped[char_set_size] = { false };
  assert(first.length() == second.length());
  for (int i = 0; i < first.length(); i++) {
    char f = first[i], s = second[i];
    if (!already_swapped[getIndex(f)]) {
      assert(!already_swapped[getIndex(s)]);
      float tmp;
      for (int c = 0; c < char_set_size; c++) {
        if (c == f || c == s)
          continue;
        tmp = mapping[f][c];
        mapping[f][c] = mapping[s][c];
        mapping[s][c] = tmp;
        tmp = mapping[c][first[i]];
        mapping[c][f] = mapping[c][s];
        mapping[c][s] = tmp;
      }
      tmp = mapping[f][f];
      mapping[f][f] = mapping[s][s];
      mapping[s][s] = tmp;
      tmp = mapping[f][s];
      mapping[f][s] = mapping[s][f];
      mapping[s][f] = tmp;
      swapLetters(visual_rep, f, s);
      for(vector<string> group : character_groups)
        for(string& sub_group : group)
          swapLetters(sub_group, f, s);
      already_swapped[getIndex(f)] = true;
      already_swapped[getIndex(s)] = true;
    }
    else
      assert(already_swapped[getIndex(s)]);
  }
  return true;
}
