//
// switchboard
//
// Given a node list from slurm, render an image of which nodes were and 
// were not involved in that operation
//
// (c)2023 Mark J Stock <markjstock@gmail.com>
//

#include "ryb_autocolor.h"

#include "lodepng.h"

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <cassert>
#include <iterator>

struct job_t {
  std::string name;
  std::vector<int> nodeids;
};

struct machine_t {
  // the machine name to search for
  std::string name;
  // the node hierarchy
  // a mapping function from node name to 0-indexed id
};

// data for frontier
const std::string machname = "frontier";
const int node_levels = 2;
const int num_per_level[node_levels] = {128, 74};
const int num_per_row[node_levels] = {8, 15};
const int total_num[node_levels] = {9472, 74};
// map the name of the machine to a 0-indexed, continuous index
int map_node_name(const int _n) {
  if (_n <= 9088) return _n-1;
  else if (_n >= 10113 and _n <= 10496) return _n-1025;
  else return -1;
}

/*
// data for crusher
const std::string machname = "crusher";
const int node_levels = 2;
const int num_per_level[node_levels] = {128, 2};
const int num_per_row[node_levels] = {8, 2};
const int total_num[node_levels] = {192, 2};
// map the name of the machine to a 0-indexed, continuous index
int map_node_name(const int _n) {
  if (_n <= 192) return _n-1;
  else return -1;
}
*/


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

  // read the node list file into a single large string
  // node list can come from a copy-paste, or the output from "squeue -t running"
  // ideally can we do "squeue -t running | switchboard frontier > image.png"

  // load in the test file
  std::ifstream ifs(in_file);
  assert (ifs.is_open() && "Could not open given node list file");
  std::string nodelist(std::istreambuf_iterator<char>{ifs}, {});
  ifs.close();

  // --------------------------------------------------------------------------
  // create arrays for the geometric hierarchy

  int boxszx[node_levels+1];
  int boxszy[node_levels+1];
  int boxbdr[node_levels+1];
  int boxgap[node_levels+1];
  int boxwid[node_levels+1];
  int boxhgt[node_levels+1];

  // drawing sizes per node
  boxszx[0] = 5;
  boxszy[0] = 5;
  boxbdr[0] = 1;
  boxgap[0] = 2;
  boxwid[0] = boxgap[0] + 2*boxbdr[0] + boxszx[0];
  boxhgt[0] = boxgap[0] + 2*boxbdr[0] + boxszy[0];
  // drawing sizes per block
  boxszx[1] = num_per_row[0]*boxwid[0];
  boxszy[1] = rows_needed(num_per_level[0],num_per_row[0])*boxhgt[0];
  boxbdr[1] = 1;
  boxgap[1] = boxwid[0];
  boxwid[1] = boxgap[1] + 2*boxbdr[1] + boxszx[1];
  boxhgt[1] = boxgap[1] + 2*boxbdr[1] + boxszy[1];
  // and for the whole image
  boxszx[2] = num_per_row[1]*boxwid[1];
  boxszy[2] = rows_needed(num_per_level[1],num_per_row[1])*boxhgt[1];
  boxbdr[2] = 1;
  boxgap[2] = 2*boxgap[1];
  boxwid[2] = boxgap[2] + 2*boxbdr[2] + boxszx[2];
  boxhgt[2] = boxgap[2] + 2*boxbdr[2] + boxszy[2];

  // --------------------------------------------------------------------------
  // clear the image and set the background

  unsigned int out_width = boxwid[2];
  unsigned int out_height = boxhgt[2];
  printf("Will create %d x %d image\n", out_width, out_height);
  std::vector<unsigned char> out_image;
  out_image.resize(out_width * out_height * 4);

  // fill with solid white
  const unsigned char bgcolor[4] = {255, 255, 255, 255};
  for (unsigned int i = 0; i < out_width*out_height; i++) {
    out_image[4*i+0] = bgcolor[0];
    out_image[4*i+1] = bgcolor[1];
    out_image[4*i+2] = bgcolor[2];
    out_image[4*i+3] = bgcolor[3];
  }

  // --------------------------------------------------------------------------
  // march through all levels and draw their boxes

  const unsigned char bdrcolor[4] = {192, 192, 192, 255};

  //for (int i=0; i<=node_levels; ++i) {
  for (int i=0; i<=0; ++i) {
    std::cout << "Drawing outlines for " << total_num[i] << " blocks..." << std::endl;

    if (boxbdr[i] > 0) {

    // can add openmp here if necessary
    for (int cnt = 0; cnt < total_num[i]; cnt++) {
      const int group = cnt / num_per_level[0];
      const int igroup = group % num_per_row[1];
      const int jgroup = group / num_per_row[1];
      const int node = cnt - group*num_per_level[0];
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
    }
  }
  }

  // --------------------------------------------------------------------------
  // repeatedly look for the keyword in the nodelist string and generate jobs
  std::vector<job_t> jobs;

  std::cout << "Parsing nodelist..." << std::endl;

  size_t pos = 0;
  while ((pos = nodelist.find(machname, pos)) != std::string::npos) {
    //std::cout << "found substring at position " << pos << std::endl;
    // advance past the substring
    pos += machname.length();
    //std::cout << ", next char is (" << nodelist.at(pos) << ")" << std::endl;
    // advance only if next character is a [
    if (nodelist.at(pos) == '[') ++pos;
    //std::cout << ", next char is (" << nodelist.at(pos) << ")" << std::endl;

    // start a potential new job
    job_t newjob;
    newjob.name = "job";
    bool keep_going = true;
    bool is_single = true;

    while (keep_going) {
      //std::cout << "continuing to read..." << std::endl;

      // quit if there's no more characters
      if (pos == nodelist.size()) {
        //std::cout << "  reached end of string" << std::endl;
        keep_going = false;

      // if next char is a digit, find the number
      } else if (std::isdigit(nodelist.at(pos))) {
        //std::cout << "  next char is digit, reading number" << std::endl;

        // read numbers 0..9 and build the nodeid
        int nodeid = 0;
        while (std::isdigit(nodelist.at(pos))) {
          nodeid = nodeid*10 + (nodelist.at(pos) - '0');
          ++pos;
          if (pos == nodelist.size()) break;
          //std::cout << ", next char is (" << nodelist.at(pos) << ")" << std::endl;
        }
        if (is_single) {
          // if we're in single mode, add this one node to the list
          //std::cout << "  adding nodeid (" << nodeid << ")" << std::endl;
          newjob.nodeids.push_back(map_node_name(nodeid));
        } else {
          // the last char was a dash, add all nodes inclusive to the list
          auto lastnodeid = newjob.nodeids[newjob.nodeids.size()-1];
          //std::cout << "  adding nodes " << lastnodeid+1 << " to " << map_node_name(nodeid) << std::endl;
          for (int thisid = lastnodeid+1; thisid <= map_node_name(nodeid); ++thisid) {
            newjob.nodeids.push_back(thisid);
          }
          is_single = true;
        }

      // if it's a dash, get ready to read another number
      } else if (nodelist.at(pos) == '-') {
        //std::cout << "  next char is dash, setting flag" << std::endl;
        is_single = false;
        ++pos;

      // if it's a comma, get ready to read another number
      } else if (nodelist.at(pos) == ',') {
        //std::cout << "  next char is comma, expecting another number" << std::endl;
        ++pos;

      // and if it's a bracket, we're done
      } else if (nodelist.at(pos) == ']') {
        //std::cout << "  next char is bracket, we're done with this entry" << std::endl;
        keep_going = false;
        ++pos;

      } else {
        keep_going = false;
      }
    }

    // add to list
    jobs.push_back(newjob);
  }

  // --------------------------------------------------------------------------
  // march through active jobs and draw them

  // set drawing parameters
  const bool overwrite_border = true;

  std::cout << "Drawing active nodes..." << std::endl;
  for (auto job : jobs) {

    // get a color for this job
    unsigned char color[4];
    (void) get_next_color(color);

    // now march through all participating nodes and color their boxes
    for (auto nodeid : job.nodeids) {

      // convert node name/number to 0-index (already done!)
      const int nodeidx = nodeid;

      if (nodeidx < 0) continue;

      const int group = nodeidx / num_per_level[0];
      const int igroup = group % num_per_row[1];
      const int jgroup = group / num_per_row[1];
      const int node = nodeidx - group*num_per_level[0];
      const int inode = node % num_per_row[0];
      const int jnode = node / num_per_row[0];

      // pixel index of top left corner
      const int bdr = (overwrite_border ? 0 : boxbdr[0]);
      const int idx = (boxgap[2]/2 + jgroup*boxhgt[1] + boxgap[1]/2 + jnode*boxhgt[0] + boxgap[0]/2 + bdr)*boxwid[2]
                     + boxgap[2]/2 + igroup*boxwid[1] + boxgap[1]/2 + inode*boxwid[0] + boxgap[0]/2 + bdr;

      // and the size of the box to draw
      const int xwid = boxszx[0] + (overwrite_border ? 2*boxbdr[0] : 0);
      const int yhgt = boxszy[0] + (overwrite_border ? 2*boxbdr[0] : 0);

      // draw the block of color
      for (int y=0; y<yhgt; ++y) {
        const int py = idx + y*boxwid[2];
        for (int x=0; x<xwid; ++x) {
          const int px = py + x;
          for (int c=0; c<4; ++c) out_image[4*px+c] = color[c];
        }
      }
    }
  }

  // --------------------------------------------------------------------------
  // output to a new png

  unsigned int error = lodepng::encode(out_file, out_image, out_width, out_height);
  //if there's an error, display it
  if (error) std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
}

