//
// gen_node_list.h
//
// Generate a fake nodelist to mimic capacity computing
//
// (c)2023 Mark J Stock <markjstock@gmail.com>
//

#pragma once

#include "switchboard.h"

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <cassert>
#include <iterator>


//
// generate a fake nodelist - frame list
//
void gen_nodelist (std::vector<frame_t>& _frames, const int _nnodes, const int _nframes) {

  assert(_nframes<100000 && "Too many frames asked for");

  // create list of ongoing jobs
  std::vector<job_t> jobs(_nnodes);
  // and a list of all nodes, and which jobs are on them
  std::vector<int> node_to_job(_nnodes);

  for (int j=0; j<_nnodes; ++j) {
    jobs[j].jobid = j;
    jobs[j].nodeids.push_back(j);
    node_to_job[j] = j;
  }
  int newjobid = _nnodes;

  // now there is a job on every node

  // create a random number generator
  std::random_device rdev;    // Will be used to obtain a seed for the random number engine
  static std::mt19937 rgen(rdev());   // Standard mersenne_twister_engine seeded with rd()
  std::uniform_real_distribution<float> unif_real(0.0,1.0);
  //unif_real(rgen)


  // march over output images
  for (int i=0; i<_nframes; ++i) {
    frame_t newframe;

    // generate the name
    char framename[20];
    sprintf(framename, "frame_%05d.png", i);
    newframe.name = framename;

    // add all current jobs
    newframe.jobs = jobs;

    // add to the frames list
    _frames.push_back(newframe);

    // now march through all active jobs, killing some
    for (auto it = jobs.begin(); it != jobs.end(); ) {
      const int job_code = 1 + (it->jobid % 13);
      const float kill_chance = 1.f/(job_code*job_code);	// (so 1 to 1/81)
      if (unif_real(rgen) < kill_chance) {
        // tell the owning proc that it's free
        const int owning_proc = it->nodeids[0];
        node_to_job[owning_proc] = -1;
        // kill this job
        it = jobs.erase(it);
      } else {
        // advance the iterator
        ++it;
      }
    }

    // and march through unused nodes, adding some
    for (int j=0; j<_nnodes; ++j) {
      if (node_to_job[j] < 0) {
        if (unif_real(rgen) < 0.25) {
          // start a new job here
          job_t newjob;
          newjob.jobid = newjobid++;
          newjob.nodeids.push_back(j);
          jobs.push_back(newjob);
          node_to_job[j] = newjob.jobid;
        }
      }
    }
  }

}

