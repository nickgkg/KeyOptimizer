#include "stdafx.h"
#include "PolygramFrequency.h"
#include <cstdlib>
#include <assert.h>
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std;
static inline unsigned char getIndex(const char c) {
  if (c < 0 || c >= 128)
    return 0;
  return c;
}
static inline unsigned char getChar(const char i) {
  return i;
}
PolygramFrequency::PolygramFrequency(int max_length_polygram) 
{
  int cap = char_set_size;
  f.resize(max_length_polygram);
  for (int i = 0; i < f.size(); ++i) {
    f[i].resize(cap);
    for (int j = 0; j < f[i].size(); j++)
      f[i][j] = 0;
    cap *= char_set_size;
  }
}
void PolygramFrequency::populatePolygrams(char file[]) {
  const long double startTime = time(0);
  std::ifstream is(file);
  int history_length = f.size();
  vector<char> history(history_length);
  int count = history_length;
  char c;
  while (is.get(c)) {
    if (c == 0)
      break;
    char index = getIndex(c);
    history[count%history_length] = index;
    int polygram_length = 1;
    for (vector<int>& freqN : f) {
      int index = 0;
      int multiplier = 1;
      for (int i = 0; i < polygram_length; i++) {
        index += multiplier * history[(count - i)%history_length];
        multiplier *= char_set_size;
      }
      ++freqN[index];
      ++polygram_length;
    }
    ++count;
    ++total;
  }
  is.close();
}
int PolygramFrequency::getFrequency(int length, vector<int>& arr) {
  assert(length <= f.size());
  int index = 0;
  for (int i = 0; i < length; ++i) {
    index *= char_set_size;
    index += arr[i];
  }
  return f[length-1][index];
}
void PolygramFrequency::logFrequencies(int n) {
  int arr[char_set_size] = { 0 };
  for (int i = 0; i < char_set_size; i++)
    arr[i] = i;
  vector<int> v(arr, arr + sizeof(arr) / sizeof(arr[0]));
  if (n > 0) {
    vector<int>& g1 = this->f[0];
    vector<int>& g2 = this->f[1];
    sort(v.begin(), v.end(), [&g1](int a, int b) {return g1[a] > g1[b]; });
    for (int i = 0; i < char_set_size; i++) {
      int i2 = v[i];
      if (g1[i2] > 0) {
        std::cout << (i + 1) << ' ' << getChar(i2) << '=' << g1[i2] << std::endl;
        if (n > 1) {
          for (int j = 0; j < char_set_size; j++) {
            if (g2[i2 * char_set_size + j] > 0)
              std::cout << getChar(i2) << getChar(j) << '=' << g2[i2 * char_set_size + j] << ' ';
          }
          std::cout << std::endl;
        }
      }
    }
  }
}
int PolygramFrequency::getMaxLengthPolygram() {
  return f.size();
}
int PolygramFrequency::getTotalCharacters() {
  return total;
}