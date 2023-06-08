//
// ryb_autocolor
//
// Create and maintain a collection of well-spaced colors using the RYB space
//
// (c)2023 Mark J Stock <markjstock@gmail.com>
//

#include <vector>
#include <array>
#include <map>
#include <random>
#include <iostream>

// colors stored as float32 internally
// but retrieved as a variety of types (unsigned char, for 8-bit applications)
// values are r, g, b, age
//std::vector<std::array<float,3>> all_colors;

// map between unique jobid (key) and xyz in unit cube (value)
std::map<int,std::array<float,4>> job_to_xyz;

// empty out the vector
void reset_color_palette() {
  job_to_xyz.clear();
  // and fill in the black and white colors again
  job_to_xyz[-2] = std::array<float,4>({1.f,1.f,1.f,0.f});
  job_to_xyz[-1] = std::array<float,4>({0.f,0.f,0.f,0.f});
}

// increment the age counter for each xyz triple
// and check for ones that have been unused for too long and release them
void age_all_colors() {
  std::cout << "aging all keys\n";
  for (auto it = job_to_xyz.begin(); it != job_to_xyz.end(); ) {
    // just age the positive-keyed entries, -1 and -2 are always reserved (see above)
    if (it->first > -1) it->second[3] += 1.f;

    // now check vs. threshold
    if (it->second[3] > 20.f) {
      std::cout << "  erasing key " << it->first << '\n';
      it = job_to_xyz.erase(it);
    } else {
      ++it;
    }
  }

}

// take a 3d unit cube position and create an RGBA color
void triple_to_color(std::array<float,4>& _xyz, unsigned char* _c) {

  // expand it toward the edges
  std::array<float,3> pt;
  for (int i=0; i<3; ++i) pt[i] = _xyz[i]*_xyz[i]*(3.f-2.f*_xyz[i]);

  // assume pt is now in ryb coords, now convert to rgb via linear interpolation
  float w[8] = {pt[0]*pt[1]*pt[2],
                pt[0]*pt[1]*(1.f-pt[2]),
                pt[0]*(1.f-pt[1])*pt[2],
                pt[0]*(1.f-pt[1])*(1.f-pt[2]),
                (1.f-pt[0])*pt[1]*pt[2],
                (1.f-pt[0])*pt[1]*(1.f-pt[2]),
                (1.f-pt[0])*(1.f-pt[1])*pt[2],
                (1.f-pt[0])*(1.f-pt[1])*(1.f-pt[2])};

  static const float r[8] = {0.2,   1.0, 0.5, 1.0, 0.0,  1.0, 0.163, 1.0};
  static const float g[8] = {0.094, 0.5, 0.0, 0.0, 0.66, 1.0, 0.373, 1.0};
  static const float b[8] = {0.0,   1.0, 0.5, 0.0, 0.2,  0.0, 0.6,   1.0};

  float rgb[3] = {0.0, 0.0, 0.0};
  for (int i=0; i<8; ++i) rgb[0] += r[i]*w[i];
  for (int i=0; i<8; ++i) rgb[1] += g[i]*w[i];
  for (int i=0; i<8; ++i) rgb[2] += b[i]*w[i];
  //printf("  becomes rgb %g %g %g\n", rgb[0], rgb[1], rgb[2]);

  // finally convert it to unsigned chars
  _c[0] = (unsigned int)(rgb[0]*255.999);
  _c[1] = (unsigned int)(rgb[1]*255.999);
  _c[2] = (unsigned int)(rgb[2]*255.999);
  _c[3] = 255;
  return;
}

// check table for existing color, return new one if new key/jobid
void get_next_color(const int _id, unsigned char* _c) {
  //std::cout << "  looking for color for jobid " << _id << std::endl;

  if (auto search = job_to_xyz.find(_id); search != job_to_xyz.end()) {
    std::cout << "    found key " << search->first << '\n';
    // recalculate the color
    triple_to_color(search->second, _c);
    // and reset the age back to zero
    search->second[3] = 0.f;
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
  std::uniform_real_distribution<float> unif_real(0.0,1.0);

  // just get a random color first
  std::array<float,4> pt({unif_real(rgen), unif_real(rgen), unif_real(rgen), 0.f});

  if (not job_to_xyz.empty()) {
    float farthest_dist = 0.0;
    float closest[3] = {0.0, 0.0, 0.0};

    // try random points for one that is far from all the others
    for (int i=0; i<10000; ++i) {
      // get a 3-tuple
      float p[3] = {unif_real(rgen), unif_real(rgen), unif_real(rgen)};
      //printf("random color is %g %g %g\n", p[0], p[1], p[2]);

      // how far is this from all other points?
      float closest_dist = 3.0;
      for (const auto& testval : job_to_xyz) {
        std::array<float,4> testpt = testval.second;
        const float distsq = std::pow(p[0]-testpt[0],2) +
                             std::pow(p[1]-testpt[1],2) +
                             std::pow(p[2]-testpt[2],2);
        if (distsq < closest_dist) {
          closest_dist = distsq;
        }
      }

      if (closest_dist > farthest_dist) {
        //printf("  is %g away from all other colors\n", std::sqrt(closest_dist));
        farthest_dist = closest_dist;
        closest[0] = p[0];
        closest[1] = p[1];
        closest[2] = p[2];
      }
    }

    // use the one that is the farthest from all others
    pt[0] = closest[0];
    pt[1] = closest[1];
    pt[2] = closest[2];
  }
  //printf("picked color %g %g %g\n", pt[0], pt[1], pt[2]);

  // now pt is a random point in the unit cube

  // save this position to the map
  job_to_xyz[_id] = pt;
  std::cout << "    added entry for key " << _id << std::endl;

  // convert it to rgba unsigned chars
  triple_to_color(pt, _c);

  return;
}

// get a unique color with no key/jobid
void get_next_color(unsigned char* _c) {
  static int key = 0;
  (void)get_next_color(key, _c);
  ++key;
}

