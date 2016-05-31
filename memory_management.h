#ifndef _mem__
#define _mem_
#include <iostream>
#include <iterator>
#include <fstream>
#include <list>
#include "generator.h"

using namespace std;

class Memory_segment{
        int space;
        char flag; //e for empty t for taken;
        int vpid;
        int process;
        int time;
    public:
        Memory_segment(int a=0){flag='e';space=a;vpid=-1;process=0;time=0;}
        void set_space(int a){space=a;}                                     //sets space
        int get_space(void){return space;}                                  //return space
        int& get_process(void){return process;}                             //return space taken from the process
        void change_flag(void){if (flag=='e') flag='t'; else flag='e';}     //changes the flag from empty to take and reverse
        char get_flag(void){return flag;}                                   //returns the flag
        int& retvpid(void){return vpid;}                                    //return the vpid in the segment
        int& rettime(void){return time;}
};

class Memory{
        int full_space;
        int free_space;
        int _usable;
        int _usable2;
        int l;
        int u;
        char c;
        long int residence_times;
        long int request_sizes;
        int total_allocations;
        int total_dallocations;
        list<Memory_segment> memory ;
        list<Memory_segment>::iterator last_it;
    public:
        Memory(int a,char ch='u',int lo=0);
        //is size of the memory
        int insert_pid(int n,int vpid,char c);                                                          //n is the size of the process ,vpid the vpid, c algorithm
        //for entry 'f' for frist-fit 'n' for next fit 'b' for buddy
        int remove_pid(int vpid);                                                                       //removes the vp from the memory and releases space char 'o' for next/best fit 'u' for buddies
        int swap(int n,int vpid,char c);                                                                //does swap_in or swap_out dempending of the state in memory
        int freespace(void){return free_space;}
        int spacetaken(void){return full_space-free_space;}
        int usable(void){return _usable;}
        int usable2(void);
        list<Memory_segment>& ret_list(void){return memory;}
        void print(ofstream &a,int time);
        void plus(void);
        int numofe(void);
        long int ret_res(void){return residence_times;}
        long int ret_totala(void){return total_allocations;}
        long int ret_req(void){return request_sizes;}
        long int ret_totald(void){return total_dallocations;}
};

class waiting_queue_elem{
    message a;
    int time;
    int time_in_queue;
    public:
    waiting_queue_elem(int vpid,int size,string op,int tim=0):a(vpid,size,op){time=tim;time_in_queue=0;}
    void up(void){time++;}                                                                                  //time until next event +1
    void upt(void){time_in_queue++;}                                                                        //time in queu +1
    void down(void){time--;}                                                                                //time until next event -1
    bool zero(void){return time==0;}                                                                        //time == 0 returns true
    message ret_message(void){return a;}                                                                    //returns the containing message
    int ret_time(void){return time;}                                                                        //returns the time
    int ret_timet(void){return time_in_queue;}                                                              //returns the time_in_queue
};

class waiting_queue_ops_elem{
    list<waiting_queue_elem> a;                                         //list of waiting queue elements (operations supended and in queue for a vp)
    int vpid;                                                           //vpid of the process for which the messages are suspended
    public:
    waiting_queue_ops_elem(int p){vpid=p;}                              //p is the vpid of the vp
    void insert (waiting_queue_elem b);                                 //inserts a new qaiting queu elem
    list <waiting_queue_elem>& ret_list(void){return a;}                //returns the list of waiting_queue_elems
    int ret_vpid(void){return vpid;}                                    //returns the vpid of the vp
};

class waiting_queue{
    list<waiting_queue_ops_elem> ops;
    list<waiting_queue_elem> vpids_w;
    public:
        void insert_message(message a,bool forcedinw=false);                  //inserts the message in the suitable queue
        bool vpid_in_w(int vpid);                                       //checks if vpid is in vpids_w
        bool vpid_in_ops(int vpid);
        bool vpid_in_w_or_ops(int vpid);                                //checks if vpid if in vpid_w or ops
        void remove_vpid_from_w(int vpid);                              //removes message from the vpid_w
        void remove_message_from_ops(int vpid);                         //removes message from ops
        void advance(void);                                             //advances timers in ops (+1 if operation in queue/else -1 )
        void print(ofstream &a,int i);
        list<waiting_queue_elem>& ret_waiting_messages(void){return vpids_w;} //returns vpids_w list
        list<waiting_queue_ops_elem>& ret_waitings_ops(void){return ops;}     //return ops list
};

#endif
