#include "stdafx.h"
#include <assert.h>
#include <string>
#include <iostream>
#include <ctime>
#include <vector>
#include "PolygramFrequency.h"
#include "KeyLayout.h"

using namespace std;

void printVisual(string visual) {
  for (int i = 0; i < visual.length(); i++)
    if (visual[i] == '\n')
      std::cout << "\\n";
    else if (visual[i] == '\t')
      std::cout << "\\t";
    else if (visual[i] == '\b')
      std::cout << "\\b";
    else if (visual[i] == '\\')
      std::cout << "\\\\";
    else
      std::cout << visual[i];
}
void maxInputWeights1to1(KeyLayout l, string a, string b, float effort) {
  assert(a.length() == b.length());
  for (int i = 0; i < a.length(); ++i) {
    l.maxInputWeights({ a[i], '\0' }, {b[i], '\0'}, effort);
  }
}

int main()
{
  PolygramFrequency freq (2);
  freq.populatePolygrams("code.txt");
  freq.logFrequencies(0);
  
  srand(time(NULL));
  KeyLayout lay (freq);
  string left_normal = "`123456qwertasdfgzxcvb\t", left_shift = "~!@#$%^QWERTASDFGZXCVB\2";
  string right_normal = "7890-=yuiop[]\\hjkl;'nm,./\n\b", right_shift = "&*()_+YUIOP{}|HJKL:\"NM<>?\3\4";
  string left = left_normal + left_shift, right = right_normal + right_shift;
  string all = left + right;
  //init with base effort of 0 to 6
  lay.addInputWeights(all, "\\|\b",6);
  lay.addInputWeights(all, "`1~!",5);
  lay.addInputWeights(all, "6=^+",4.5);
  lay.addInputWeights(all, "2347890-]@#$&*()_}",4);
  lay.addInputWeights(all, "5b%B",3.5);
  lay.addInputWeights(all, "yY",3);
  lay.addInputWeights(all, "tT\n",2.5);
  lay.addInputWeights(all, "qweruiopgh'zxcvnm,./QWERUIOPGH\"ZXCVNM<>?",2);
  lay.addInputWeights(all, " asdfjkl;ASDFJKL:", 0);//uneeded since it's initialized to 0
  //make keys near to eachother and double tapping easier (0 effort)
  vector<vector<string>> locality = {
    {"`\tqaz","~\tQAZ"},//left pinky
    {"12wsx","!@WSX"},//left ring
    {"3edc","#EDC"},//left middle
    {"4rfv","$RFV","5tgb","%TGB"}, {"56t","%^T"},//left pointer
    {" "},//thumbs
    {"yhn","YHN","ujm","UJM"}, {"y7u","Y&U"},//right_pointer
    {"8ik,","*IK<"},//right middle
    {"9ol.","(OL>"},//right ring
    {"0p;",")P:","-['","_{\""}, {";/'",":?\""},{"-[","_{","=]","+}"}, {"\n]\\\n","\n}|\n"}, {"'\n","\"\n"},//right pinky
  };
  for (vector<string> vec : locality) {
    int count = 0;
    for (string st : vec) {
      maxInputWeights1to1(lay, st, st, 0);//hitting key twice (ex. h->h)
      string prior = st.substr(0, st.length() - 1), next = st.substr(1, st.length() - 1);
      maxInputWeights1to1(lay, prior, next, 0);//hitting key below (ex. h->y)
      maxInputWeights1to1(lay, next, prior, 0);//hitting key above (ex. h->n)
      for (int bit = 1; bit < vec.size(); bit *= 2) {
        int count2 = count ^ bit;
        maxInputWeights1to1(lay, st, vec[count2], 0);// hitting key twice with shift one of the times (ex. h->H) or hitting key beside (ex. h->j)
      }
      count++;
    }
  }
  //alternating hand difficulty = 0
  lay.addInputWeights(left, right, -2);
  lay.addInputWeights(right, left, -2);
  //make using shift more difficult
  {
    //use shift on opposite side
    lay.minInputWeights(left_normal, left_shift, 1);
    lay.minInputWeights(right_normal, right_shift, 1);
    //use shift on same side
    lay.minInputWeights(left_normal, right_shift, 2.5);
    lay.minInputWeights(right_normal, left_shift, 2.5);
  }
  //make sure there is no negative effort (only possible from alternating hand difficulty)
  lay.minInputWeights(all, all, 0);
  //set alphabetic characters to move with their lowercase/uppercase partner
  lay.setMustMoveInNDimGroups(2, "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM");
  //set initial pairing configuration
  lay.addNDimGroups(vector<string>{left_normal + right_normal, left_shift + right_shift});
  lay.setVisualRep({ "`1234567890-=\b","\tqwertyuiop[]\\","asdfghjkl;'\n","zxcvbnm,./","~!@#$%^&*()_+\b","\tQWERTYUIOP{}|","ASDFGHJKL:\"\n","ZXCVBNM<>?"," "});
  string best_visual_rep = lay.getVisualRep();
  float best_score = lay.forceUpdateScore();
  printVisual(best_visual_rep);
  cout << " " << best_score << endl;
  for(int repeat = 0; repeat < 10; repeat++){
    //shuffle
    for (int c = 0; c < all.length(); ++c)
      lay.swapCharacters({ all[c], '\0' }, { all[rand() % all.length()], '\0'});
    float lowest = lay.adaptiveUpdateScore();
    int count;
    do {
      count = 0;
      //generate random iterating order
      string order = best_visual_rep;
      for (int i = 0; i < order.length(); ++i) {
        char tmp = order[i];
        int j = rand() % order.length();
        order[i] = order[j];
        order[j] = tmp;
      }
      for (int i = 0; i < order.length(); ++i) {
        char a = order[i];
        if (a == '\1')
          continue;
        for (int j = i + 1; j < order.length(); ++j) {
          char b = order[j];
          if (b == '\1')
            continue;
          if (lay.swapCharacters({ a, '\0' }, { b, '\0' })) {
            float score = lay.adaptiveUpdateScore();
            if (score < lowest) {
              lowest = score;
              count++;
            }else {
              lay.swapCharacters({ a, '\0' }, { b, '\0' });
            }
          }
        }
      }
    } while (count != 0);//swap until no more improvements can be made
    //sanity check
    float score = lay.adaptiveUpdateScore();
    assert(lowest == score);
    string visual_rep = lay.getVisualRep();
    printVisual(visual_rep);
    cout << " " << lowest << endl;
    if (lowest < best_score) {
      best_visual_rep = visual_rep;
      best_score = lowest;
    }
  }
  //reprint the best answer
  cout << endl;
  printVisual(best_visual_rep);
  cout << " " << best_score << endl;
  string wait;
  cin >> wait;
}
