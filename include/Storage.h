#ifndef __STORAGE_H__
#define __STORAGE_H__
#include <Arduino.h>    //for using String type

#define CARD_SIZE_LIMIT_MB 15000
#define MIN_CHUNK_SIZE 10

/*
 * The fomrmat for file name is: YYYYMMDD.txt
 */

class Storage {
private:
    bool resume;
    long curr_read_pos;
    String curr_read_file;
    int curr_chunk_size;
    bool mount_success;
    void remove_oldest_file();

public:
    bool init_storage(); 
    bool write_data(String timenow, String data);
    String read_data(void);
    void mark_data(void);
    long get_unsent_data(void);
};


#endif