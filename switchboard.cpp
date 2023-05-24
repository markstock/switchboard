//
// switchboard
//
// Given a node list from slurm, render an image of which nodes were and 
// were not involved in that operation
//
// (c)2023 Mark J Stock <markjstock@gmail.com>
//

#include "lodepng.h"

#include <iostream>
#include <cstdio>


// data for frontier
int const node_levels = 2;
const int node_hierarchy[node_levels] = {128, 74};
const int num_per_row[node_levels] = {8, 19};
// and some sort of mapping of node id to these groups?
int map_frontier(const int _n) {
  return _n-1;
}

// how many rows are needed?
int rows_needed(const int _n, const int _nperrow) {
  return (_n+_nperrow-1)/_nperrow;
}

//
// entry and exit
//
int main(int argc, char *argv[])
{
  const char* in_file = argc > 1 ? argv[1] : "nodelist";
  const char* out_file = "out.png";

  // load in the test file
  std::vector<unsigned char> in_image; //the raw pixels
  unsigned int in_width, in_height;
  unsigned int error = lodepng::decode(in_image, in_width, in_height, in_file);
  //if there's an error, display it
  if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

  // colors
  unsigned char bdrcolor[4] = {192, 192, 192, 255};
  unsigned char usecolor[4] = {32, 32, 96, 255};

  // create new image data arrays
  int boxszx[node_levels+1];
  int boxszy[node_levels+1];
  int boxbdr[node_levels+1];
  int boxgap[node_levels+1];
  int boxwid[node_levels+1];
  int boxhgt[node_levels+1];

  // drawing sizes per node
  boxszx[0] = 4;
  boxszy[0] = 4;
  boxbdr[0] = 1;
  boxgap[0] = 2;
  boxwid[0] = boxgap[0] + 2*boxbdr[0] + boxszx[0];
  boxhgt[0] = boxgap[0] + 2*boxbdr[0] + boxszy[0];
  // drawing sizes per block
  boxszx[1] = num_per_row[0]*boxwid[0];
  boxszy[1] = rows_needed(node_hierarchy[0],num_per_row[0])*boxhgt[0];
  boxbdr[1] = 0;
  boxgap[1] = boxwid[0];
  boxwid[1] = boxgap[1] + 2*boxbdr[1] + boxszx[1];
  boxhgt[1] = boxgap[1] + 2*boxbdr[1] + boxszy[1];
  // and for the whole image
  boxszx[2] = num_per_row[1]*boxwid[1];
  boxszy[2] = rows_needed(node_hierarchy[1],num_per_row[1])*boxhgt[1];
  boxbdr[2] = 0;
  boxgap[2] = 2*boxgap[1];
  boxwid[2] = boxgap[2] + 2*boxbdr[2] + boxszx[2];
  boxhgt[2] = boxgap[2] + 2*boxbdr[2] + boxszy[2];
  unsigned int out_width = boxwid[2];
  unsigned int out_height = boxhgt[2];
  printf("Will create %d x %d image\n", out_width, out_height);

  std::vector<unsigned char> out_image;
  out_image.resize(out_width * out_height * 4);
  // fill with solid white
  for (unsigned int i = 0; i < out_width*out_height; i++) {
    out_image[4*i+0] = 255;
    out_image[4*i+1] = 255;
    out_image[4*i+2] = 255;
    out_image[4*i+3] = 255;
    //out_image[4*i+3] = 0;
  }

  int nnodes = 1;
  for (int i=0; i<node_levels; ++i) nnodes *= node_hierarchy[i];
  std::cout << "Drawing outlines for " << nnodes << " nodes..." << std::endl;

  // march through all nodes and draw their boxes
  if (boxbdr[0] > 0) {
  // can add openmp here if necessary
  for (int cnt = 0; cnt < nnodes; cnt++) {
    const int group = cnt / node_hierarchy[0];
    const int igroup = group % num_per_row[1];
    const int jgroup = group / num_per_row[1];
    const int node = cnt - group*node_hierarchy[0];
    const int inode = node % num_per_row[0];
    const int jnode = node / num_per_row[0];

    //std::cout << "Node " << cnt << " is " << node << " in group " << group << "\n";
    //printf("Node %d is %d (%d x %d) in group %d (%d x %d)\n", cnt, node, inode, jnode, group, igroup, jgroup);

    // pixel index of top left corner
    const int idx = (boxgap[2]/2 + jgroup*boxhgt[1] + boxgap[1]/2 + jnode*boxhgt[0] + boxgap[0]/2)*boxwid[2]
                   + boxgap[2]/2 + igroup*boxwid[1] + boxgap[1]/2 + inode*boxwid[0] + boxgap[0]/2;

    // draw the top and bottom bars
    for (int y=0; y<boxbdr[0]; ++y) {
      const int py = idx + y*boxwid[2];
      for (int x=0; x<(boxszx[0]+2*boxbdr[0]); ++x) {
        const int px = py + x;
        for (int c=0; c<4; ++c) out_image[4*px+c] = bdrcolor[c];
      }
    }
    for (int y=0; y<boxbdr[0]; ++y) {
      const int py = idx + (y+boxbdr[0]+boxszy[0])*boxwid[2];
      for (int x=0; x<(boxszx[0]+2*boxbdr[0]); ++x) {
        const int px = py + x;
        for (int c=0; c<4; ++c) out_image[4*px+c] = bdrcolor[c];
      }
    }

    // draw the sides
    for (int y=0; y<boxszy[0]; ++y) {
      const int py = idx + (y+boxbdr[0])*boxwid[2];
      for (int x=0; x<boxbdr[0]; ++x) {
        const int px = py + x;
        for (int c=0; c<4; ++c) out_image[4*px+c] = bdrcolor[c];
      }
      for (int x=0; x<boxbdr[0]; ++x) {
        const int px = py + boxbdr[0] + boxszx[0] + x;
        for (int c=0; c<4; ++c) out_image[4*px+c] = bdrcolor[c];
      }
    }

    //size_t src_addr = 4*in_width*y + 4*x;
    // now use bins
    //float angle = (hue_bins*in_hsv[src_addr+0]/255)/(float)hue_bins * 6.28318531;
    // idea: consider scaling the displacement by the value or saturation
    //unsigned int destx = x + band + hue_displace * cos(angle);
    //unsigned int desty = y + band + hue_displace * sin(angle);
    //size_t dest_addr = 4*out_width*desty + 4*destx;
    // need to be able to splat this smoothly, but need blending to do that
    // need to understand how to do blending to make this work
    //out_image[dest_addr + 0] = in_image[src_addr + 0];
    //out_image[dest_addr + 1] = in_image[src_addr + 1];
    //out_image[dest_addr + 2] = in_image[src_addr + 2];
    //out_image[dest_addr + 3] = in_image[src_addr + 3];
  }
  }

  // march through all groups and draw their borders
  if (boxbdr[1] > 0) {
  }

  // now march through all participating nodes and color their boxes
  //if (numactive > 0) {
  //}

  // output to a new png
  error = lodepng::encode(out_file, out_image, out_width, out_height);
  //if there's an error, display it
  if (error) std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
}

