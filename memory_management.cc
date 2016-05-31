#include <iostream>
#include <iterator>
#include <list>
#include <cmath>
#include <fstream>
#include "memory_management.h"
#include "generator.h"
using namespace std;

double log2(double a){
    return log10(a)/log10(2);
}

Memory::Memory(int a,char ch,int lo){
    free_space=a;
    total_allocations=0;
    total_dallocations=0;
    residence_times=0;
    request_sizes=0;
    Memory_segment b(full_space=a);
    _usable=free_space;
    memory.push_front(a);
    last_it=memory.begin();
    l=lo;
    u=log2(a);
    c=ch;
}

int Memory::remove_pid(int vpid){
    list<Memory_segment>::iterator it,it2,it3,it_print;
    Memory_segment a;
    bool flag=false,flag2=false;
    /* cout<<"Before delete"<<endl; */
    /* for(it_print=memory.begin();it_print!=memory.end();++it_print){ */
        /* cout<< (*it_print).get_space()<<" "<< (*it_print).retvpid() <<"|"<< (*it_print).get_flag() << " "; */
    /* } */
    /* cout<<endl; */
    if(c=='n'||c=='b'){
        for(it=memory.begin();it!=memory.end();it++){
            if(flag==false&& (*it).get_flag()=='e'){
                it2=it;
                flag=true;
            }
            if((*it).retvpid()==vpid){
                (*it).change_flag();
                 total_dallocations++;
                free_space+=it->get_space();
                _usable+=it->get_space();
                flag2=true;
                 break;
            }
            if((*it).get_flag()=='t'&& flag==true){
                flag=false;
            }
        }
        if (!flag2){
            return -2;
        }
        if(!flag){
            it2=it;
        }
        /* cout<<(*it2).get_space()<<endl; */
        for(;it2!=memory.end() && (*it2).get_flag()!='t';){
            it=it2;
            it++;
            a.set_space(a.get_space()+(*it2).get_space());
            if(last_it==it2){
                last_it=it;
            }
            residence_times+=it2->rettime();
            memory.erase(it2);
            it2=it;
        }
        memory.insert(it,a);
    }
    else if(c=='u'){
        int newspace;
        int size=0;
        for(it=memory.begin();it!=memory.end();it++){
            if(it->retvpid()==vpid){
                 it->change_flag();
                 it->retvpid()=-1;
                 residence_times+=it->rettime();
                 it->rettime()=0;
                 free_space+=it->get_process();
                 total_dallocations++;
                 _usable+=it->get_space();
                 it->get_process()=0;
            }
        }
        for(it=memory.begin();it!=memory.end();){
            if(it->get_flag()=='e'){
                it2=it;
                if((size/it->get_space())%2==1){
                    it2--;
                    it3=it;
                    it3++;
                }
                else{
                    it2++;
                    it3=it2;
                    it3++;
                }
                if(it2->get_flag()=='e'&&it2->get_space()==it->get_space()){
                    newspace=it->get_space()*2;
                    memory.erase(it);
                    memory.erase(it2);
                    a.set_space(newspace);
                    memory.insert(it3,a);
                    it=memory.begin();
                    size=0;
                }
                else{
                    size+=it->get_space();
                    it++;
                    }
            }
            else{
                size+=it->get_space();
                it++;
            }
        }
    }
    /* cout<<"After delete"<<endl; */
    /* for(it_print=memory.begin();it_print!=memory.end();++it_print){ */
        /* cout<< (*it_print).get_space() <<"|"<< (*it_print).get_flag() << " "; */
    /* } */
    /* cout<<endl; */
    return 0;
}

int Memory::usable2(void){
    int usable_2=0;
    for(list<Memory_segment>::iterator it=memory.begin();it!=memory.end();++it){
        if(it->get_flag()=='e'){
            /* cout<<it->get_space()<<endl; */
            usable_2+=it->get_space()*it->get_space();
        }
    }
    return usable_2;
}

int Memory::numofe(void){
    int numofe=0;
    for(list<Memory_segment>::iterator it=memory.begin();it!=memory.end();++it){
        if(it->get_flag()=='e'){
            numofe++;;
        }
    }
    return numofe;
}

void Memory::plus(void){
    for(list<Memory_segment>::iterator it=memory.begin();it!=memory.end();++it){
        if(it->get_flag()=='t'){
            it->rettime()++;
        }
    }
}

int Memory::insert_pid(int n,int vpid,char c){
    list<Memory_segment>::iterator it,it2,it_print;
    Memory_segment a,b;
    int to_part;
    bool flag=false;
    if(c=='n'||c=='b'){
        if(c=='n'){
            for(it=last_it;it!=memory.end();it++){
                if((*it).get_space()>=n && (*it).get_flag()!='t'){
                    flag=true;
                    break;
                }
            }
            if(flag!=true)
                for(it=memory.begin();it!=last_it;++it){
                    if((*it).get_space()>=n && (*it).get_flag()!='t'){
                        flag=true;
                        break;
                    }
                }
        }
        else if(c=='b'){
            int min=full_space;
            it2=memory.begin();
            for(it=memory.begin();it!=memory.end();it++){
                if((*it).get_space()>=n && (*it).get_flag()!='t'){
                    if(it->get_space()-n<min){
                        min=(*it).get_space()-n;
                        it2=it;
                        flag=true;
                    }
                }
            }
            it=it2;
        }
        /* cout<<"Before input"<<endl; */
        /* for(it_print=memory.begin();it_print!=memory.end();++it_print){ */
            /* cout<< (*it_print).get_space()<<" "<< (*it_print).retvpid() <<" "<< it->get_process()<<"|"<< (*it_print).get_flag() << " || "; */
        /* } */
        /* cout<<endl; */
        if(flag==false){
            return -1;
        }
        to_part=(*it).get_space();
        if((*it).get_space()==n){
            (*it).change_flag();
            (*it).get_process()=n;
            (*it).retvpid()=vpid;
            last_it=it;
        }
        if((to_part=(*it).get_space())>n){
            it2=it;
            it2++;
            memory.erase(it);
            a.set_space(n);
            a.retvpid()=vpid;
            a.change_flag();
            b.set_space(to_part-n);
            if(it2!=memory.begin() && !memory.empty()){
                memory.insert(it2,a);
                last_it=it2;
                memory.insert(it2,b);
            }
            else {
                memory.insert(memory.begin(),b);
                memory.insert(memory.begin(),a);
                last_it=memory.begin();
            }
        }
    }
    else if(c=='u'){
        bool found=false;
        bool state=true;
        bool split=false;
        /* cout<<"Before input"<<endl; */
        /* for(it_print=memory.begin();it_print!=memory.end();++it_print){ */
            /* cout<< (*it_print).get_space()<<" "<< (*it_print).retvpid() <<" "<< (*it_print).get_process()<<"|"<< (*it_print).get_flag() << " || "; */
        /* } */
        /* cout<<endl; */
        while(state){
            found=false;
            split=false;
            for(list<Memory_segment>::iterator it=memory.begin();it!=memory.end();it++){
                if((log2(it->get_space())==ceil((log2(n)))||((log2(it->get_space())==l)&& it->get_space()>=n))&& it->get_flag()!='t'){
                    flag=true;
                    it->change_flag();
                    it->retvpid()=vpid;
                    it->get_process()=n;
                    /* cout<<"After input"<<endl; */
                    /* for(it_print=memory.begin();it_print!=memory.end();++it_print){ */
                        /* cout<< (*it_print).get_space()<<" "<< (*it_print).retvpid() <<" "<< it_print->get_process()<<"|"<< (*it_print).get_flag() << " || "; */
                    /* } */
                    /* cout<<endl; */
                    free_space-=it->get_space();
                    _usable-=it->get_space();
                    total_allocations++;
                    request_sizes+=n;
                    return 0;
                }
            }
            if(!found){
                for(list<Memory_segment>::iterator it2,it=memory.begin();it!=memory.end();){
                    if(log2(it->get_space())>ceil((log2(n)))&&log2(it->get_space()) != l && it->get_flag()!='t'){
                        to_part=it->get_space();
                        memory.erase(it++);
                        a.set_space(to_part/2);
                        b.set_space(to_part/2);
                        it2=it;
                        if(it2!=memory.begin() && !memory.empty()){
                            memory.insert(it2,a);
                            memory.insert(it2,b);
                            it2--;
                            it2--;
                            it=it2;
                        }
                        else{
                            memory.push_front(a);
                            memory.push_front(b);
                            it=memory.begin();
                        }
                        split=true;
                        break;
                    }
                    else{
                         it++;
                    }
                }
                if(split==false){return -1;}
            }
        }
    }
    /* cout<<"After input"<<endl; */
    /* for(it_print=memory.begin();it_print!=memory.end();++it_print){ */
        /* cout<< (*it_print).get_space()<<" "<< (*it_print).retvpid() <<"|"<< (*it_print).get_flag() << " "; */
    /* } */
    /* cout<<endl; */
    free_space-=n;
    _usable-=n;
    total_allocations++;
    request_sizes+=n;
    return 0;
}

void Memory::print(ofstream &a,int time){
    a<<"-----Memory state-----"<<endl;
    for(list<Memory_segment>::iterator it_print=memory.begin();it_print!=memory.end();++it_print){
        a<<"seg_sz:"<< (*it_print).get_space()<<" vpid:"<< (*it_print).retvpid() <<" vp_s: "<< it_print->get_process()<<" seg_s:"<< (*it_print).get_flag() << " || ";
    }
    a<<endl;
    a<<"Time - memory Product efficiency: "<<((double)residence_times*request_sizes/total_allocations)/(full_space*time)<<endl;
}

void waiting_queue_ops_elem::insert(waiting_queue_elem b){
    a.push_back(b);
}

void waiting_queue::insert_message(message a,bool forcedinw){
    list<waiting_queue_elem>::iterator itm;
    list<waiting_queue_ops_elem>::iterator ito;
    int time=0;
    if(forcedinw){
        waiting_queue_elem temp(a.ret_pid(),a.ret_size(),a.ret_string());
        vpids_w.push_back(temp);
        return;
    }
    if(vpid_in_w_or_ops(a.ret_pid())){
        if(vpid_in_w(a.ret_pid())){
            if(vpid_in_ops(a.ret_pid())){
                for(ito=ops.begin();ito!=ops.end();ito++){
                    if(ito->ret_vpid()==a.ret_pid()){
                        time=ito->ret_list().back().ret_timet();
                        waiting_queue_elem temp(a.ret_pid(),a.ret_size(),a.ret_string(),time);
                        ito->insert(temp);
                        return;
                    }
                }
            }
            else{
                for(itm=vpids_w.begin();itm!=vpids_w.end();itm++){
                    if(itm->ret_message().ret_pid()==a.ret_pid()){
                        time=itm->ret_time();
                        waiting_queue_elem temp(a.ret_pid(),a.ret_size(),a.ret_string(),time);
                        waiting_queue_ops_elem b(a.ret_pid());
                        ops.push_front(b);
                        ops.begin()->insert(temp);
                        return;
                    }
                }
            }
        }
        else{
            if(vpid_in_ops(a.ret_pid())){
                for(ito=ops.begin();ito!=ops.end();ito++){
                    if(ito->ret_vpid()==a.ret_pid()){
                        time=ito->ret_list().back().ret_timet();
                        waiting_queue_elem temp(a.ret_pid(),a.ret_size(),a.ret_string(),time);
                        ito->insert(temp);
                        return;
                    }
                }
            }
            else{
                waiting_queue_elem temp(a.ret_pid(),a.ret_size(),a.ret_string(),1);
                vpids_w.push_back(temp);
            }
        }
    }
    else{
        waiting_queue_elem temp(a.ret_pid(),a.ret_size(),a.ret_string(),1);
        vpids_w.push_back(temp);
        return;
    }
}

void waiting_queue::advance(void){
    list<waiting_queue_elem>::iterator itm;
    list<waiting_queue_ops_elem>::iterator ito;
    bool flag=false;
    /* cout<<"vpids time"<<endl; */
    for(itm=vpids_w.begin();itm!=vpids_w.end();itm++){
        itm->up();
        /* cout<<"id :"<<itm->ret_message().ret_pid()<<" time: "<<itm->ret_time()<<"| "; */
    }
    /* cout<<""<<endl; */
    for(ito=ops.begin();ito!=ops.end();ito++){
        if(vpid_in_w(ito->ret_vpid())){
            flag=true;
        }
        else{
            flag=false;
        }
        /* cout<<"id :"<<ito->ret_vpid()<<endl; */
        for(itm=ito->ret_list().begin();itm!=ito->ret_list().end();itm++){
            itm->upt();
            /* cout<<itm->ret_time()<<" "; */
            if(!flag&& itm==ito->ret_list().begin()){
                 itm->down();
            }
        }
        /* cout<<endl; */
    }
}

void waiting_queue::remove_vpid_from_w(int vpid){
    for(list<waiting_queue_elem>::iterator it=vpids_w.begin();it!=vpids_w.end();){
        if(it->ret_message().ret_pid()==vpid){
            vpids_w.erase(it++);
            break;
        }
        it++;
    }
}

void waiting_queue::remove_message_from_ops(int vpid){
    for(list<waiting_queue_ops_elem>::iterator it=ops.begin() ,it2; it!=ops.end();){
        it2=it;
        it2++;
        if(it->ret_vpid()==vpid){
            it->ret_list().pop_front();
            if(it->ret_list().empty()){
                ops.erase(it);
            }
            break;
        }
        it=it2;
    }
}

bool waiting_queue::vpid_in_w(int vpid){
    for(list<waiting_queue_elem>::iterator it=vpids_w.begin();it!=vpids_w.end();it++){
        if(it->ret_message().ret_pid()==vpid){
            return true;
        }
    }
    return false;
}

bool waiting_queue::vpid_in_ops(int vpid){
    for(list<waiting_queue_ops_elem>::iterator it=ops.begin();it!=ops.end();it++){
        if(it->ret_vpid()==vpid){
            return true;
        }
    }
    return false;
}

bool waiting_queue::vpid_in_w_or_ops(int vpid){
    return (vpid_in_w(vpid)||vpid_in_ops(vpid));
}

int Memory::swap(int n,int vpid,char c){
    for(list<Memory_segment>::iterator it=memory.begin();it!=memory.end();it++){
        if(it->retvpid()==vpid){
            remove_pid(vpid);
            return 1;
        }
    }
    return insert_pid(n,vpid,c);
}

void waiting_queue::print(ofstream &a,int i){
    a<<"time :"<<i<<" Messages in queue"<<endl;
    int number=0;
    list<waiting_queue_elem>::iterator itm;
    list<waiting_queue_ops_elem>::iterator ito;
    for(itm=vpids_w.begin();itm!=vpids_w.end();itm++){
        /* a<<"id :"<<itm->ret_message().ret_pid()<<" time: "<<itm->ret_time()<<" || "; */
        number++;
    }
    /* if(number!=0) */
        /* a<<"END"<<endl; */
    for(ito=ops.begin();ito!=ops.end();ito++){
        number++;
        /* a<<"id : "<<ito->ret_vpid()<<" time : "<<ito->ret_list().begin()->ret_time()<<" || "; */
    }
    a<<"\nTotal number of messages in queue or suspended : "<<number<<endl;

}


//----------------------MAIN USED FOR TESTING MEMORY ALGORITHMS
/* int main(void){ */
/*     Memory memory(1024,5,log2(1024)); */
/*     int a,b,c; */
/*     while(1){ */
/*         cout<<"1 insert.\n" << "2 delete.\nSomething else to Quit."<<endl; */
/*         cin>>a; */
/*         if(a==1){ */
/*             cout<<"Enter space , id"<<endl; */
/*             cin>> b >> c; */
/*             memory.insert_pid(b,c,'u'); */
/*         } */
/*         else if(a==2){ */
/*              cin>>b; */
/*              memory.remove_pid(b,'u'); */
/*         } */
/*         else{ */
/*             break; */
/*         } */
/*     } */
/*         return 0; */
/* } */
