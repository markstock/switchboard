//
// switchboard.h
//
// Given a node list from slurm, render an image of which nodes were and 
// were not involved in that operation
//
// (c)2023 Mark J Stock <markjstock@gmail.com>
//

#pragma once

#include <vector>
#include <string>

struct job_t {
  //std::string name;
  int jobid;
  std::vector<int> nodeids;
};

struct frame_t {
  std::string name;
  std::vector<job_t> jobs;
};

//struct machine_t {
  // the machine name to search for
  //std::string name;
  // the node hierarchy
  // a mapping function from node name to 0-indexed id
//};

const std::string nextfilekey = "file";		// keyword to look for to start a new file

