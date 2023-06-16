//
// hpe_autocolor
//
// Create and maintain a collection of hpe colors
//
// (c)2023 Mark J Stock <markjstock@gmail.com>
//

#pragma once

#include <vector>
#include <array>
#include <map>
#include <random>
#include <iostream>


// the hpe colors, including black
//std::array<std::array<unsigned char,3>,10> hpecolors = {{0,0,0},{255,255,255},{1,169,130},
//                                                        {13,82,101},{50,218,200},{127,249,226},
//                                                        {118,48,234},{193,64,255},{255,131,0},
//                                                        {254,201,1}};
std::array<std::array<unsigned char,3>,10> hpecolors = {0,0,0,255,255,255,1,169,130,
                                                        13,82,101,50,218,200,127,249,226,
                                                        118,48,234,193,64,255,255,131,0,
                                                        254,201,1};

// map between unique jobid (key) and xyz in unit cube (value)
std::map<int,std::array<unsigned char,3>> job_to_rgb;

// empty out the vector
void reset_color_palette() {
  job_to_rgb.clear();
}

// increment the age counter for each xyz triple
// and check for ones that have been unused for too long and release them
void age_all_colors() {
}

// to color a failed job, use black
void get_failed_color(unsigned char* _c) {
  _c[0] = hpecolors[0][0];
  _c[1] = hpecolors[0][1];
  _c[2] = hpecolors[0][2];
  _c[3] = 255;
}

// check table for existing color, return new one if new key/jobid
void get_next_color(const int _id, unsigned char* _c) {
  //std::cout << "  looking for color for jobid " << _id << std::endl;

  if (auto search = job_to_rgb.find(_id); search != job_to_rgb.end()) {
    //std::cout << "    found key " << search->first << '\n';
    // retrieve the color
    _c[0] = search->second[0];
    _c[1] = search->second[1];
    _c[2] = search->second[2];
    _c[3] = 255;
    return;
  }

  //std::cout << "    not found\n";

  // get the random generator started
  std::random_device rdev;    // Will be used to obtain a seed for the random number engine
  static std::mt19937 rgen(rdev());   // Standard mersenne_twister_engine seeded with rd()
  static bool initialized = false;

  if (not initialized) {
    rgen.seed(12345);
    initialized = true;

    // ensure that we're empty
    reset_color_palette();
  }

  // just get a random color, but not white or black
  std::uniform_int_distribution<int> unif_int(2,9);
  std::array<unsigned char,3> col(hpecolors[unif_int(rgen)]);

  // save this position to the map
  job_to_rgb[_id] = col;
  //std::cout << "    added entry for key " << _id << " with color " << (int)col[0] << " " << (int)col[1] << " " << (int)col[2] << std::endl;

  // convert it to rgba unsigned chars
  _c[0] = col[0];
  _c[1] = col[1];
  _c[2] = col[2];
  _c[3] = 255;

  return;
}

// get a unique color with no key/jobid
void get_next_color(unsigned char* _c) {
  static int key = 0;
  (void)get_next_color(key, _c);
  ++key;
}

