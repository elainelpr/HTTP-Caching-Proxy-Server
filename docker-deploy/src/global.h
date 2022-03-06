//
// Created by Xinyu Guo on 2/22/22.
//

#ifndef HW2_FINAL_GLOBAL_H
#define HW2_FINAL_GLOBAL_H

#include <fstream>
extern std::ofstream LOG_FILE;
extern pthread_mutex_t logfile_mutex;
extern pthread_mutex_t cache_mutex;
#endif //HW2_FINAL_GLOBAL_H
