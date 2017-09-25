#include "pstree.h"
#include <fstream>
#include <dirent.h>

std::map<int, PsNode*> PsNode::int_to_pid;

PsNode::PsNode(int pid_)
{
    if(int_to_pid.count(pid_) == 0) {
        this->pid_ = pid_;
        int_to_pid[pid_] = this;
        //read from file ppid
        int ppid;
        std::ifstream ps_file;
        ps_file.open(std::string("/proc/")+std::to_string(pid_)+"/stat");
        std::string garb;
        ps_file >> garb >> garb >> garb >> ppid;
        ps_file.close();
        if(int_to_pid.count(ppid) == 0)
            int_to_pid[ppid] = new PsNode(ppid);
        this->parent = int_to_pid[ppid];
        this->parent->children.push_back(this);
    }
    else {
        std::cerr << "Node already existing for PID : " << pid_ << std::endl;
    }
}

int PsNode::ppid()
{
    if(parent)
        return parent->pid_;
    return -1;
}

std::vector<int> PsNode::children_pid()
{
    std::vector<int> result;
    for(int i=0; i<children.size(); i++)
        result.push_back(children[i]->pid_);
    return result;
}

void PsNode::deleteAll()
{
    for(std::map<int,PsNode*>::iterator iter = int_to_pid.begin(); iter != int_to_pid.end(); ++iter)
    {
        delete(iter->second);
        int_to_pid.erase(iter->first);
    }
}

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

void PsNode::createAll()
{
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir ("/proc")) != NULL) {
      /* print all the files and directories within directory */
      while ((ent = readdir (dir)) != NULL) {
        if(is_number(std::string(ent->d_name))) {
            int id = stoi(std::string(ent->d_name));
            if(int_to_pid.count(id) == 0)
                int_to_pid[id] = new PsNode(id);
        }
      }
      closedir (dir);
    } else {
      /* could not open directory */
      std::cerr << "Could not open : /proc" << std::endl;
    }
}

int PsNode::pid()
{
    return pid_;
}
