//
// ryb_autocolor
//
// Create and maintain a collection of well-spaced colors using the RYB space
//
// (c)2023 Mark J Stock <markjstock@gmail.com>
//

#include <vector>
#include <array>
#include <random>

// colors stored as float32 internally
// but retrieved as a variety of types (unsigned char, for 8-bit applications)
std::vector<std::array<float,3>> all_colors;

// placeholder
//void get_next_color(unsigned char* _c) {
//  _c[0] = 32;
//  _c[1] = 32;
//  _c[2] = 160;
//  _c[3] = 255;
//  return;
//}

// the real one
void get_next_color(unsigned char* _c) {

  // get the random generator started
  std::random_device rdev;    // Will be used to obtain a seed for the random number engine
  static std::mt19937 rgen(rdev());   // Standard mersenne_twister_engine seeded with rd()
  static bool initialized = false;

  if (not initialized) {
    rgen.seed(12345);
    initialized = true;
  }
  std::uniform_real_distribution<float> unif_real(0.0,1.0);

  // just get a random color first
  float pt[3] = {unif_real(rgen), unif_real(rgen), unif_real(rgen)};
  // or just dial in a single color
  //float pt[3] = {0.125, 0.125, 0.625};

  if (not all_colors.empty()) {
    float farthest_dist = 0.0;
    float closest[3] = {0.0, 0.0, 0.0};

    // try random points for one that is far from all the others
    for (int i=0; i<100; ++i) {
      // get a 3-tuple
      float p[3] = {unif_real(rgen), unif_real(rgen), unif_real(rgen)};
      //printf("random color is %g %g %g\n", p[0], p[1], p[2]);

      // how far is this from all other points?
      float closest_dist = 3.0;
      //float closest_pt[3] = {0.0, 0.0, 0.0};
      for (auto testpt : all_colors) {
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

  // save it as a color
  all_colors.push_back(std::array<float,3>({pt[0],pt[1],pt[2]}));

  // expand it toward the edges
  float color[3] = {pt[0], pt[1], pt[2]};

  // and convert to ryb

  // and convert it to unsigned chars
  _c[0] = (unsigned int)(color[0]*255.999);
  _c[1] = (unsigned int)(color[1]*255.999);
  _c[2] = (unsigned int)(color[2]*255.999);
  _c[3] = 255;
  return;
}
