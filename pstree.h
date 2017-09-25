/*
 * pstree.h
 *
 * Class to inspect process id tree, reading infos from the /proc directory
 * Construct the tree with PsNode::createAll()
 * Find infos from pid with the map PsNode::int_to_pid
 * Free the tree with PsNode::deleteAll()
 *
 */

#ifndef PSTREE_H
#define PSTREE_H

#include <iostream>
#include <vector>
#include <map>

class PsNode
{
public:
    static void deleteAll();
    static void createAll();
    static std::map<int, PsNode*> int_to_pid;

    int pid();
    int ppid();
    std::vector<int> children_pid();

    PsNode * parent;
    std::vector<PsNode*> children;
private:
    PsNode(int);
    int pid_;
};

#endif // PSTREE_H
