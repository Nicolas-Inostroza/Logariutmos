#ifndef SEARCH_H
#define SEARCH_H

#include "manejodisco.h"

void range_search_B_disk(DiskManager &dm, int node_idx, int l, int u, std::vector<std::pair<int,float>> &out);
std::vector<std::pair<int,float>> range_search_Bplus_disk(DiskManager &dm, int root_idx, int l, int u);

#endif
