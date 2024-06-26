//
// switchboard
//
// Given a node list from "squeue -t running", render an image of which jobs were running
// on which nodes, using well-separated colors from the RYB color space
//
// (c)2023 Mark J Stock <markjstock@gmail.com>
//

#include "switchboard.h"
#include "ryb_autocolor.h"

#include "lodepng.h"
#include "CLI11.hpp"

#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <cassert>
#include <iterator>


//
// data for frontier, exascale machine at ORNL
//
// machine-specific
const std::string machname = "frontier";		// name to look for in nodelist
const int nlevels = 3;							// number of levels of hierarchy
const int num_per_level[nlevels] = {128, 74, 1};// number of items in each level of hierarchy
const int total_num[nlevels] = {9472, 74, 1};	// total number of items in each level
int map_node_name(const int _n) {				// function to map the name of the machine to a 0-indexed, continuous index
  if (_n <= 9088) return _n-1;
  else if (_n >= 10113 and _n <= 10496) return _n-1025;
  else return -1;
}
// drawing-specific
const int base_size[2] = {5, 5};				// size of interior of finest block in pixels
const int num_per_row[nlevels] = {8, 15, 1};	// number of items to draw in one row in each level
const int block_border[nlevels] = {1, 1, 1};	// width of drawn border in each level in pixels
const int block_gap[nlevels] = {2, 8, 16};		// width of white-space gap between each item at each level in pixels

/*
// data for crusher
const std::string machname = "crusher";
const int nlevels = 3;
const int num_per_level[nlevels] = {128, 2, 1};
const int total_num[nlevels] = {192, 2, 1};
// map the name of the machine to a 0-indexed, continuous index
int map_node_name(const int _n) {
  if (_n <= 192) return _n-1;
  else return -1;
}
const int base_size[2] = {5, 5};				// size of interior of finest block in pixels
const int num_per_row[nlevels] = {8, 2, 1};
const int block_border[nlevels] = {1, 1, 1};	// width of drawn border in each level in pixels
const int block_gap[nlevels] = {2, 8, 16};		// width of white-space gap between each item at each level in pixels
*/


// how many rows are needed?
int rows_needed(const int _n, const int _nperrow) {
  return (_n+_nperrow-1)/_nperrow;
}

//
// entry and exit
//
int main(int argc, char *argv[]) {

  std::cout << "switchboard v1.0\n";

  // set up command line arg definitions
  CLI::App app{"Generate hierarchical block rendering of jobs on a supercomputer"};
  std::string nodefn = "nodelist";
  app.add_option("-n,--nodelist", nodefn, "name of nodelist text file");
  std::string pngfn = "out.png";
  app.add_option("-o,--output", pngfn, "name of output png file");

  // finally parse
  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  }

  // node list can come from a copy-paste, or the output from "squeue -t running"
  // ideally can we do "squeue -t running | switchboard frontier > image.png"

  // --------------------------------------------------------------------------
  // create arrays for the geometric hierarchy

  int boxszx[nlevels];
  int boxszy[nlevels];
  int boxbdr[nlevels];
  int boxgap[nlevels];
  int boxwid[nlevels];
  int boxhgt[nlevels];

  // set drawing sizes per node/block/image
  for (int i=0; i<nlevels; ++i) {

    if (i==0) {
      boxszx[i] = base_size[0];
      boxszy[i] = base_size[1];
    } else {
      boxszx[i] = num_per_row[i-1]*boxwid[i-1];
      boxszy[i] = rows_needed(num_per_level[i-1],num_per_row[i-1])*boxhgt[i-1];
    }
    boxbdr[i] = block_border[i];
    boxgap[i] = block_gap[i];
    boxwid[i] = boxgap[i] + 2*boxbdr[i] + boxszx[i];
    boxhgt[i] = boxgap[i] + 2*boxbdr[i] + boxszy[i];
  }

  // --------------------------------------------------------------------------
  // allocate and initialize the base image

  unsigned int out_width = boxwid[nlevels-1];
  unsigned int out_height = boxhgt[nlevels-1];
  printf("Will create %d x %d image\n", out_width, out_height);
  std::vector<unsigned char> base_image;
  base_image.resize(out_width * out_height * 4);

  // fill with solid white
  const unsigned char bgcolor[4] = {255, 255, 255, 255};
  for (unsigned int i = 0; i < out_width*out_height; i++) {
    base_image[4*i+0] = bgcolor[0];
    base_image[4*i+1] = bgcolor[1];
    base_image[4*i+2] = bgcolor[2];
    base_image[4*i+3] = bgcolor[3];
  }

  // --------------------------------------------------------------------------
  // march through all levels and draw their boxes

  const unsigned char bdrcolor[4] = {192, 192, 192, 255};

  //for (int i=0; i<nlevels; ++i) {
  for (int i=0; i<1; ++i) {
    std::cout << "Drawing outlines for " << total_num[i] << " blocks at level " << i << std::endl;

    if (boxbdr[i] > 0) {

    // can add openmp here if necessary
    for (int cnt = 0; cnt < total_num[i]; cnt++) {

      const int group = cnt / num_per_level[i];
      const int igroup = group % num_per_row[i+1];
      const int jgroup = group / num_per_row[i+1];

      const int node = cnt - group*num_per_level[i];
      const int inode = node % num_per_row[i];
      const int jnode = node / num_per_row[i];

      //std::cout << "Node " << cnt << " is " << node << " in group " << group << "\n";
      //printf("Node %d is %d (%d x %d) in group %d (%d x %d)\n", cnt, node, inode, jnode, group, igroup, jgroup);

      // pixel index of top left corner
      const int col = boxgap[2]/2  +  jgroup*boxhgt[1] + boxgap[1]/2  +  jnode*boxhgt[0] + boxgap[0]/2;
      const int row = boxgap[2]/2  +  igroup*boxwid[1] + boxgap[1]/2  +  inode*boxwid[0] + boxgap[0]/2;
      const int idx = col*out_width + row;

      // draw the top and bottom bars
      for (int y=0; y<boxbdr[i]; ++y) {
        const int py = idx + y*out_width;
        for (int x=0; x<(boxszx[i]+2*boxbdr[i]); ++x) {
          const int px = py + x;
          for (int c=0; c<4; ++c) base_image[4*px+c] = bdrcolor[c];
        }
      }
      for (int y=0; y<boxbdr[i]; ++y) {
        const int py = idx + (y+boxbdr[i]+boxszy[i])*out_width;
        for (int x=0; x<(boxszx[i]+2*boxbdr[i]); ++x) {
          const int px = py + x;
          for (int c=0; c<4; ++c) base_image[4*px+c] = bdrcolor[c];
        }
      }

      // draw the sides
      for (int y=0; y<boxszy[i]; ++y) {
        const int py = idx + (y+boxbdr[i])*out_width;
        for (int x=0; x<boxbdr[i]; ++x) {
          const int px = py + x;
          for (int c=0; c<4; ++c) base_image[4*px+c] = bdrcolor[c];
        }
        for (int x=0; x<boxbdr[i]; ++x) {
          const int px = py + boxbdr[i] + boxszx[i] + x;
          for (int c=0; c<4; ++c) base_image[4*px+c] = bdrcolor[c];
        }
      }
    }
    }
  }

  // draw a default color for every node
  if (false) {
    // get a color for this job
    const unsigned char unused[4] = {231, 231, 231, 255};

    // now march through all participating nodes and color their boxes
    for (int nodeidx = 0; nodeidx < total_num[0]; nodeidx++) {

      const int group = nodeidx / num_per_level[0];
      const int igroup = group % num_per_row[1];
      const int jgroup = group / num_per_row[1];
      const int node = nodeidx - group*num_per_level[0];
      const int inode = node % num_per_row[0];
      const int jnode = node / num_per_row[0];

      // pixel index of top left corner
      const int bdr = 0;
      const int col = boxgap[2]/2  +  jgroup*boxhgt[1] + boxgap[1]/2  +  jnode*boxhgt[0] + boxgap[0]/2 + bdr;
      const int row = boxgap[2]/2  +  igroup*boxwid[1] + boxgap[1]/2  +  inode*boxwid[0] + boxgap[0]/2 + bdr;
      const int idx = col*out_width + row;

      // and the size of the box to draw
      const int xwid = boxszx[0] + 2*boxbdr[0];
      const int yhgt = boxszy[0] + 2*boxbdr[0];

      // draw the block of color
      for (int y=0; y<yhgt; ++y) {
        const int py = idx + y*out_width;
        for (int x=0; x<xwid; ++x) {
          const int px = py + x;
          for (int c=0; c<4; ++c) base_image[4*px+c] = unused[c];
        }
      }
    }
  }

  // --------------------------------------------------------------------------
  // repeatedly look for the keyword in the nodelist string and generate jobs

  std::cout << "Parsing nodelist..." << std::endl;

  // store all data in a vector of frames
  std::vector<frame_t> frames;

  // read the node list file into a single large string (could be >10MB per day)
  std::ifstream ifs(nodefn);
  assert (ifs.is_open() && "Could not open given node list file");
  std::cout << "Reading nodelist..." << std::endl;
  std::string nodelist(std::istreambuf_iterator<char>{ifs}, {});
  ifs.close();

  // start a potential new frame
  frame_t newframe;

  std::string nextframename = pngfn;
  int nextjobid = 1;

  size_t pos = 0;
  while (pos != std::string::npos) {

    // look for one of any particular search strings - or a line end
    auto nextnodename = nodelist.find(machname, pos);
    auto nextfilename = nodelist.find(nextfilekey, pos);
    auto nextnewline = nodelist.find('\n', pos);
    //std::cout << "positions of next machine " << nextnodename << " filename " << nextfilename << "  and newline " << nextnewline << std::endl;
    // must look for a line end because then the first number on the next line must be the jobID!

    if (nextfilename == nextnodename) {
      // no more machines or files - both are npos (end of "file") - we're done
      pos = nextfilename;

    } else if (nextnewline < nextnodename and nextnewline < nextfilename) {
      // a newline is next - read it and look for the first number on the new line
      pos = nextnewline;
      // advance past all whitespace
      while (std::isspace(nodelist.at(pos))) ++pos;
      //std::cout << "  next char is (" << nodelist.at(pos) << ")" << std::endl;

      // read numbers 0..9 and build the jobid
      int jobid = 0;
      while (std::isdigit(nodelist.at(pos))) {
        jobid = jobid*10 + (nodelist.at(pos) - '0');
        ++pos;
        if (pos == nodelist.size()) break;
        //std::cout << ", next char is (" << nodelist.at(pos) << ")" << std::endl;
      }
      //std::cout << "  found jobid (" << jobid << ")" << std::endl;
      nextjobid = jobid;
      //exit(1);

    } else if (nextfilename < nextnodename and nextfilename < nextnewline) {
      // next actionable keyword is a new image file name

      // if there were jobs, push all of those into a new frame
      if (not newframe.jobs.empty()) {
        std::cout << "Finishing frame with " << newframe.jobs.size() << " jobs" << std::endl;
        newframe.name = nextframename;
        frames.push_back(newframe);

        // clear out the temp frame's jobs list
        newframe.jobs.clear();
      }

      pos = nextfilename;

      // this is a new file name, save it until we need it
      // advance past the substring
      pos += nextfilekey.length();
      // advance past all whitespace
      while (std::isspace(nodelist.at(pos))) ++pos;
      //std::cout << "  next char is (" << nodelist.at(pos) << ")" << std::endl;
      // find the next non-space character
      const size_t firstchar = pos;
      while (not std::isspace(nodelist.at(pos))) ++pos;
      const size_t lastchar = pos;
      // and read the filename into a string
      nextframename = nodelist.substr(firstchar, lastchar-firstchar);
      std::cout << "Read filename (" << nextframename << ")" << std::endl;

    } else {
      // next actionable keyword is a new job, parse it
      pos = nextnodename;

      //std::cout << "found machine name at position " << pos << std::endl;
      // advance past the substring
      pos += machname.length();
      //std::cout << ", next char is (" << nodelist.at(pos) << ")" << std::endl;
      // advance only if next character is a [
      if (nodelist.at(pos) == '[') ++pos;
      //std::cout << ", next char is (" << nodelist.at(pos) << ")" << std::endl;

      // start a potential new job
      job_t newjob;
      //newjob.name = "job";
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
      } // end while (keep_going)

      // add to list
      std::cout << "  adding job " << nextjobid << " with " << newjob.nodeids.size() << " nodes" << std::endl;
      newjob.jobid = nextjobid;
      newframe.jobs.push_back(newjob);

      // increment jobid in case it isn't given
      nextjobid++;

    } // end adding job
  }

  // put whatever's on the stack into the last/only frame
  std::cout << "Finishing last frame with " << newframe.jobs.size() << " jobs" << std::endl;
  newframe.name = nextframename;
  frames.push_back(newframe);

  // --------------------------------------------------------------------------
  // march through active frames and jobs and draw them

  // set drawing parameters
  const bool overwrite_border = true;

  // reset the color palette (later we will maintain it so same jobs have constant color)
  reset_color_palette();

  // loop over all frames in vector
  for (auto frame : frames) {

    // prepare the new output image as a copy of the baseline image
    std::vector<unsigned char> out_image = base_image;

    std::cout << "Drawing active nodes into " << frame.name << std::endl;
    for (auto job : frame.jobs) {

      // get a color for this job
      std::array<unsigned char,4> color;

      // check database for this jobid - return its color
      (void) get_next_color(job.jobid, color.data());

      // or always generate a new one
      //(void) get_next_color(color);

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
        const int col = boxgap[2]/2  +  jgroup*boxhgt[1] + boxgap[1]/2  +  jnode*boxhgt[0] + boxgap[0]/2 + bdr;
        const int row = boxgap[2]/2  +  igroup*boxwid[1] + boxgap[1]/2  +  inode*boxwid[0] + boxgap[0]/2 + bdr;
        const int idx = col*out_width + row;

        // and the size of the box to draw
        const int xwid = boxszx[0] + (overwrite_border ? 2*boxbdr[0] : 0);
        const int yhgt = boxszy[0] + (overwrite_border ? 2*boxbdr[0] : 0);

        // draw the block of color
        for (int y=0; y<yhgt; ++y) {
          const int py = idx + y*out_width;
          for (int x=0; x<xwid; ++x) {
            const int px = py + x;
            for (int c=0; c<4; ++c) out_image[4*px+c] = color[c];
          }
        }
      }
    }

    // output to a new png
    unsigned int error = lodepng::encode(frame.name.c_str(), out_image, out_width, out_height);
    //if there's an error, display it
    if (error) std::cout << "  Encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;

    // "age" each of the colors by 1
    age_all_colors();
  }
}

