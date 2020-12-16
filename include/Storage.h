#ifndef __STORAGE_H__
#define __STORAGE_H__
#include <Arduino.h>    //for using String type

class Storage {
private:
    long curr_read_pos;
    bool mount_success;
public:
    Storage();
    void write_data(String, String);
    String read_data(void);
    void mark_data(void);
    long get_unsent_data(void);
};


#endif