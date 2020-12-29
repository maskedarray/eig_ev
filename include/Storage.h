#ifndef __STORAGE_H__
#define __STORAGE_H__
#include <Arduino.h>    //for using String type
#include <SD.h>

#define CARD_SIZE_LIMIT_MB 15000
#define LOW_SPACE_LIMIT_MB 1024    //
#define MIN_CHUNK_SIZE_B 10
#define MAX_CHUNK_SIZE_B 1500

/*
 * The fomrmat for file name is: YYYYMMDD.txt this should be strictly followed
 */
//WARNING: before calling mark_data, read data should be called. Also, the read data function should
//         be checked for validity (comparing with "") before calling mark_data.

class Storage {
private:
    bool resume;
    long curr_read_pos;
    String curr_read_file;
    int curr_chunk_size;
    bool mount_success;
    void remove_oldest_file();
    String next_file(String);
    void create_header(File);
public:
    bool init_storage(); 
    bool write_data(String timenow, String data);
    String read_data(void);
    void mark_data(String timenow);
    long get_unsent_data(String timenow);       //return unsent data in MBs
};

extern Storage storage;

#endif