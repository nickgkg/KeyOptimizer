#pragma once
#include <vector>
#include <string>
#include "PolygramFrequency.h"

/**
A class to manage keymapping and evaluation functions to measure effectiveness
Keymapping is represented as effort to go from having just pressed one key to finishing pressing down another (a bigram)
ex. on keyboard effort for 'as' might be 0 while effort for 'ws' might be 1
Total effectivness is calculated by combining the effort with frequency of polygrams
Effort of polygrams larger than 2 are derived from the effort of bigrams
*/
class KeyLayout {
public:
  /**
  @param freq Frequency object which if changed forceUpdateScore() must be called before adaptiveUpdateScore()
  additionally this object will adopt the max_length_polygram value that the frequency has (higher means more accurate but exponentially longer runtime)
  */
  KeyLayout(PolygramFrequency& freq);
  /**
  Sets effort for all keypresses to 0
  */
  void resetKeymapping();
  /**
  Adds effort add to every mapping from all characters in start to any character in dest
  */
  void addInputWeights(std::string start, std::string dest, float add);
  /**
  Sets effort of every mapping from all characters in start to any character in dest to at least effort of minimum
  */
  void minInputWeights(std::string start, std::string dest, float minimum);
  /**
  Sets effort of every mapping from all characters in start to any character in dest to at most effort of maximum
  */
  void maxInputWeights(std::string start, std::string dest, float maximum);
  /**
  set groups of characters that can move together
  @param vec length of vec is number of characters in each group and each string index is a different group
  ex. {"ab\-", "AB|_"} links aA, bB, \| and -_ together in pairs
  */
  void addNDimGroups(std::vector<std::string> vec);
  /**
  set keys that must move in the groups that were specified by addNDimGroups()
  @param n the number of characters that each of the keys needs to move with
  @param keys the characters must move in groups
  ex. "abAB" but not "\-|_"
  */
  void setMustMoveInNDimGroups(int n, std::string keys);
  /**
  set a human readable text that will be swapped along with the keys
  @param vec Will be concatenated with char '\1's in between
  */
  void setVisualRep(std::vector<std::string> vec);
  void setVisualRep(std::string);
  std::string getVisualRep();
  /**
  use this if it's the first time or the related PolygramFrequency object has changed.
  @return A new score that's generated from scratch
  */
  float forceUpdateScore();
  /**
  use this instead of forceUpdateScore has some performance gains.
  @return A score that is created from using the last known score
  */
  float adaptiveUpdateScore();
  /**
  Swaps the two sets of keys that are currently related to the two sets of characters
  this swaps effort, visual representation and character_groupings
  character_groupings on keyboard '\' and '|' are grouped but '_' could swap with '|' making group '\' and '_'
  then '\_' could swap with 'aA' but not '\|' since '\|' are no longer grouped
  */
  bool swapCharacters(std::string a, std::string b);
private:
  //returns effort of the new mapping for len length polygram
  float getNewMapping(int len, std::vector<int>& chars);
  //returns effort of the current mapping for len length polygram
  float getMapping(int len, std::vector<int>& chars);
  //swaps two chars with eachother in a string
  void swapLetters(std::string& ref, char a, char b);
  PolygramFrequency& freq;
  std::string visual_rep = "";
  std::vector<std::vector<std::string>> character_groups;//current group configuration of all keys
  std::vector<std::string> should_move_character;//characters that need to move in groups with respect to character_groups var
  std::vector<std::vector<float>> mapping;//holds new effort values for m2 and consequently higher mX's
  std::vector<std::vector<float>> m;//current effort required to type this sequence
  float current_score = 0;
};