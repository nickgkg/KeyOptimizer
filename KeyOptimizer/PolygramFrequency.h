#pragma once
#include <vector>
#include <array>
/**
Data container for holding and generating frequency of polygrams
ex. keylogger output can be used to find ideal mappings for a certain person's typing tendencies
*/
class PolygramFrequency {
public :
  /**
  @param max_length_polygram This object will keep track of polygrams up to and including max_length_polygram
  */
  PolygramFrequency(int max_length_polygram);
  /**
  populate polygram frequencies using text from file f
  ex. keylogger output file
  */
  void populatePolygrams(char f[]);
  /**
  outputs debug info to cout for the frequencies
  @parameter n the max length polygram that should be outputed (maximum of 2)
  */
  void logFrequencies(int n);
  /**
  Returns frequency of a polygram
  @parameter len Polygram length you want to return
  @parameter arr Polygram you want checked (starting at index 0 and of length len)
  @return frequency of given polygram
  */
  int getFrequency(int len, std::vector<int>& arr);
  /**
  @return the max length polygram that this frequency object can handle
  */
  int getMaxLengthPolygram();
  /**
  @return total number of characters that have been used to create the current frequencies
  */
  int getTotalCharacters();
  static int const char_set_size = 128;//number of ascii keys which are supported (\0 through \127)
private:
  //total number of characters that have been input
  int total = 0;
  //polygram data structure
  std::vector<std::vector<int>> f;
};